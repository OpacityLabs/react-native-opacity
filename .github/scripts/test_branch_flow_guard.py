import io
import json
import os
import urllib.error

import unittest

import branch_flow_guard as guard

guard.linear_branch_lookup = lambda api_key, branch: None


def test_extract_keys():
    assert guard.extract_keys("adrianp/ENG-149-some-title") == [("ENG", "149", "ENG-149")]
    assert guard.extract_keys("adrianp/eng-149-some-title") == [("ENG", "149", "ENG-149")]
    assert guard.extract_keys("ENG-149") == [("ENG", "149", "ENG-149")]
    assert guard.extract_keys("dijar/DEVOPS-162-rollout") == [("DEVOPS", "162", "DEVOPS-162")]
    # A key anywhere on a boundary counts.
    assert guard.extract_keys("adrianp/fix-ENG-149-login") == [("ENG", "149", "ENG-149")]
    assert guard.extract_keys("foo/bar-ENG-7") == [("ENG", "7", "ENG-7")]
    assert guard.extract_keys("somethingENG-149") == [("SOMETHINGENG", "149", "SOMETHINGENG-149")]
    assert guard.extract_keys("plain-branch") == []
    assert guard.extract_keys("dijar/change-record-guard") == []
    assert guard.extract_keys("adrianp/ENG-149-fix-2") == [("ENG", "149", "ENG-149")]
    # Looks like a key. Linear answers that it does not exist.
    assert guard.extract_keys("release-2024") == [("RELEASE", "2024", "RELEASE-2024")]


def test_linear_branch_match_wins():
    lookup, calls = fake_lookup(set())
    by_branch = lambda api_key, branch: "ENG-149" if "eng-149" in branch.lower() else None
    # Linear links the branch: no key lookup needed.
    assert guard.check_first_hop("dev", "adrianp/fix-eng-149-login", "", "secret", lookup, branch_lookup=by_branch) == 0
    assert calls == []
    # Linear does not link it, the key does not exist either.
    assert guard.check_first_hop("dev", "adrianp/ENG-999999-nope", "", "secret", lookup, branch_lookup=by_branch) == 1
    assert calls == [("secret", "ENG", "999999")]
    # Linear does not link it, but the key exists: fallback passes.
    lookup2, calls2 = fake_lookup({("ENG", "149")})
    assert guard.check_first_hop("dev", "weird~ENG-149", "", "secret", lookup2, branch_lookup=lambda k, b: None) == 0
    assert calls2 == [("secret", "ENG", "149")]
    # No key shape at all: no API call.
    assert guard.check_first_hop("dev", "wip", "", "secret", lookup2, branch_lookup=lambda k, b: "ENG-1") == 1
    assert calls2 == [("secret", "ENG", "149")]

    def down(api_key, branch):
        raise guard.LinearError("500")

    # The branch query fails but the key exists: the key lookup is the second chance.
    lookup3, calls3 = fake_lookup({("ENG", "149")})
    assert guard.check_first_hop("dev", "adrianp/ENG-149-title", "", "secret", lookup3, branch_lookup=down) == 0
    assert calls3 == [("secret", "ENG", "149")]
    # The branch query fails and the key does not exist: fail.
    lookup4, calls4 = fake_lookup(set())
    assert guard.check_first_hop("dev", "adrianp/ENG-149-title", "", "secret", lookup4, branch_lookup=down) == 1
    assert calls4 == [("secret", "ENG", "149")]

    # Both queries fail: fail closed.
    def key_down(api_key, team, number):
        raise guard.LinearError("500")

    assert guard.check_first_hop("dev", "adrianp/ENG-149-title", "", "secret", key_down, branch_lookup=down) == 1


def fake_lookup(found):
    calls = []

    def lookup(api_key, team, number):
        calls.append((api_key, team, number))
        if (team, number) in found:
            return f"{team}-{number}"
        return None

    return lookup, calls


def test_first_hop_paths():
    lookup, calls = fake_lookup({("ENG", "149")})
    bot = "dependabot[bot]"
    assert guard.check_first_hop("dev", "dependabot/npm_and_yarn/left-pad-1.2.3", "", "", lookup, bot) == 0
    assert guard.check_first_hop("dev", "dependabot/manual-change", "", "", lookup, bot) == 0
    assert guard.check_first_hop("dev", "dependabot-preview/npm/left-pad-1.2.3", "", "", lookup, "dependabot-preview[bot]") == 0
    assert calls == []
    # The branch name alone is not proof.
    assert guard.check_first_hop("dev", "dependabot/manual-change", "", "", lookup, "dijar") == 1
    assert guard.check_first_hop("dev", "dependabot/manual-change", "", "", lookup) == 1
    assert guard.check_first_hop("dev", "nolinear/docs-typo", "fix the typo", "secret", lookup) == 0
    assert guard.check_first_hop("dev", "NoLinear/docs-typo", "fix the typo", "secret", lookup) == 0
    assert guard.check_first_hop("dev", "nolinear/ENG-999999-still-no-ticket", "described", "secret", lookup) == 0
    assert calls == []
    assert guard.check_first_hop("dev", "nolinear/docs-typo", "   ", "secret", lookup) == 1
    assert guard.check_first_hop("dev", "nolinear", "described", "secret", lookup) == 1
    assert guard.check_first_hop("dev", "nolinear/", "described", "secret", lookup) == 1
    assert guard.check_first_hop("dev", "wip-feature", "", "secret", lookup) == 1
    assert guard.check_first_hop("dev", "adrianp/ENG-149-title", "", "", lookup) == 1
    assert calls == []
    assert guard.check_first_hop("dev", "adrianp/ENG-149-title", "", "secret", lookup) == 0
    assert calls == [("secret", "ENG", "149")]
    assert guard.check_first_hop("dev", "adrianp/ENG-999999-nope", "", "secret", lookup) == 1


def test_linear_error_fails_closed():
    def lookup(api_key, team, number):
        raise guard.LinearError("401")

    assert guard.check_first_hop("dev", "adrianp/ENG-149-title", "", "secret", lookup) == 1


def test_flows_do_not_call_linear_on_later_hops():
    lookup, calls = fake_lookup({("ENG", "149")})
    assert guard.enforce("three", "main", "staging", "", "", lookup) == 0
    assert guard.enforce("three", "staging", "dev", "", "", lookup) == 0
    assert guard.enforce("three", "main", "dev", "", "", lookup) == 1
    assert guard.enforce("three", "main", "nolinear/hotfix", "desc", "secret", lookup) == 1
    assert guard.enforce("three", "staging", "nolinear/hotfix", "desc", "secret", lookup) == 1
    assert guard.enforce("three", "dev", "adrianp/ENG-149-title", "", "secret", lookup) == 0
    assert guard.enforce("two", "main", "staging", "", "", lookup) == 0
    assert guard.enforce("two", "staging", "adrianp/ENG-149-title", "", "secret", lookup) == 0
    assert guard.enforce("two", "main", "adrianp/ENG-149-title", "", "secret", lookup) == 1
    assert guard.enforce("two", "dev", "adrianp/ENG-149-title", "", "secret", lookup) == 0
    assert guard.enforce("zorya", "main", "adrianp/ENG-149-title", "", "secret", lookup) == 0
    assert guard.enforce("zorya", "staging", "adrianp/ENG-149-title", "", "secret", lookup) == 1
    assert guard.enforce("nope", "main", "staging", "", "", lookup) == 1
    assert calls


def test_back_merges_need_no_ticket():
    lookup, calls = fake_lookup(set())
    assert guard.enforce("three", "dev", "main", "", "", lookup) == 0
    assert guard.enforce("three", "dev", "staging", "", "", lookup) == 0
    assert guard.enforce("three", "staging", "main", "", "", lookup) == 0
    assert guard.enforce("two", "staging", "main", "", "", lookup) == 0
    # Forward hops stay exact.
    assert guard.enforce("three", "main", "dev", "", "", lookup) == 1
    assert guard.enforce("two", "main", "dev", "", "", lookup) == 1
    assert guard.enforce("zorya", "main", "staging", "", "secret", lookup) == 1
    assert calls == [("secret", "STAGING", "")] or calls == []


def test_fork_branches_are_not_protected_branches():
    lookup, calls = fake_lookup(set())
    # Back-merge names from a fork get no exemption.
    assert guard.enforce("three", "dev", "main", "", "", lookup, same_repo=False) == 1
    assert guard.enforce("three", "dev", "staging", "", "", lookup, same_repo=False) == 1
    assert guard.enforce("two", "staging", "main", "", "", lookup, same_repo=False) == 1
    # Forward hops from a fork are not the real promotion either.
    assert guard.enforce("three", "main", "staging", "", "", lookup, same_repo=False) == 1
    assert guard.enforce("three", "staging", "dev", "", "", lookup, same_repo=False) == 1
    assert guard.enforce("two", "main", "staging", "", "", lookup, same_repo=False) == 1
    assert calls == []
    # Feature branches from a fork still go through the first-hop rules.
    assert guard.enforce("three", "dev", "nolinear/docs", "described", "", lookup, same_repo=False) == 0
    assert guard.enforce("three", "dev", "wip", "", "secret", lookup, same_repo=False) == 1
    # The same names from this repository stay trusted.
    assert guard.enforce("three", "dev", "main", "", "", lookup, same_repo=True) == 0
    assert guard.enforce("three", "main", "staging", "", "", lookup, same_repo=True) == 0


def test_dependabot_author_from_api():
    lookup, calls = fake_lookup(set())
    assert guard.enforce("three", "dev", "dependabot/pip/requests-2.32.0", "", "", lookup, "dependabot[bot]") == 0
    assert guard.enforce("three", "dev", "dependabot/pip/requests-2.32.0", "", "", lookup, "someone") == 1
    assert calls == []


def test_staging_attestation():
    sha = "a" * 40
    other = "b" * 40
    good = f"""Staging URL: https://staging.example
Exercised: opened the dashboard
Result: the new page loaded
Tested commit: {sha}

- [x] I tested the changes currently on staging
"""
    assert guard.staging_test("dev", "feature", good, "true", "dijar") == 0
    assert guard.staging_test("main", "staging", good, "", "dijar") == 0
    assert guard.staging_test("main", "staging", good, "false", "dijar") == 0
    assert guard.staging_test("main", "staging", "", "true", "dijar", sha) == 1
    assert guard.staging_test("main", "staging", good, "true", "dijar", sha) == 0
    assert guard.staging_test("main", "staging", good, "true", "dijar", other) == 1
    assert guard.staging_test("main", "staging", good, "true", "dijar", "") == 1
    assert guard.staging_test("main", "staging", guard.ATTESTATION, "true", "dijar", sha) == 1
    bold = f"""**Staging URL:** https://staging.example
**Exercised:** clicked around
**Result:** worked
**Tested commit:** {sha}
- [X] I tested the changes currently on staging
"""
    assert guard.staging_test("main", "staging", bold, "true", "dijar", sha.upper()) == 0
    unchecked = good.replace("- [x]", "- [ ]")
    assert guard.staging_test("main", "staging", unchecked, "true", "dijar", sha) == 1
    empty_bold = f"""**Staging URL:**
**Exercised:** *
**Result:** **
**Tested commit:** {sha}
- [x] I tested the changes currently on staging
"""
    assert guard.staging_test("main", "staging", empty_bold, "true", "dijar", sha) == 1
    crlf = good.replace("\n", "\r\n")
    assert guard.staging_test("main", "staging", crlf, "true", "dijar", sha) == 0
    bad_url = good.replace("https://staging.example", "not-a-url")
    assert guard.staging_test("main", "staging", bad_url, "true", "dijar", sha) == 1
    assert guard.staging_url_ok("https://staging.example/path")
    assert guard.staging_url_ok("http://staging.internal:8443")
    assert guard.staging_url_ok("https://staging.example.com?preview=123")
    assert guard.staging_url_ok("https://staging.example.com#section")
    assert guard.staging_url_ok("https://staging.example.com/path?preview=123#section")
    assert guard.staging_url_ok("https://staging_host.internal/app")
    assert guard.staging_url_ok("http://[::1]:8080/")
    assert guard.staging_test(
        "main",
        "staging",
        good.replace("https://staging.example", "https://staging.example.com?preview=123"),
        "true",
        "dijar",
        sha,
    ) == 0
    assert not guard.staging_url_ok("not-a-url")
    assert not guard.staging_url_ok("https://")
    assert not guard.staging_url_ok("https://...")
    assert not guard.staging_url_ok("https://staging.example.com?preview=123 more")
    assert not guard.staging_url_ok("ftp://staging.example.com")
    assert not guard.staging_url_ok("staging.example.com")
    assert not guard.staging_url_ok("http://[::1")


def test_staging_record_formatting_variants():
    sha = "a" * 40
    formatted = f"""**Staging URL**: <https://staging.example.com/app>
**Exercised**: login
`Result`: fine
Tested commit: `{sha}`

- [x] I tested the changes currently on staging.
"""
    assert guard.staging_test("main", "staging", formatted, "true", "dijar", sha) == 0
    assert guard.field_value(formatted, "Staging URL") == "https://staging.example.com/app"
    assert guard.field_value(formatted, "Tested commit") == sha

    bulleted = f"""Staging URL: https://staging.example.com
Exercised:
- login
- signup
Result:
> both flows completed
Tested commit: {sha}

- [x] I tested the changes currently on staging
"""
    assert guard.field_value(bulleted, "Exercised") == "login\nsignup"
    assert guard.field_present(bulleted, "Result")
    assert guard.staging_test("main", "staging", bulleted, "true", "dijar", sha) == 0

    # A label followed directly by the next label is still empty.
    hollow = f"""Staging URL: https://staging.example.com
Exercised:
Result: fine
Tested commit: {sha}

- [x] I tested the changes currently on staging
"""
    assert not guard.field_present(hollow, "Exercised")
    assert guard.staging_test("main", "staging", hollow, "true", "dijar", sha) == 1

    # A label followed by the checkbox is empty too.
    hollow_end = f"""Staging URL: https://staging.example.com
Exercised: login
Result: fine
Tested commit:
- [x] I tested the changes currently on staging
"""
    assert guard.field_value(hollow_end, "Tested commit") == ""

    # The first non-empty match wins, so a blank template above does not hide a filled copy.
    pasted = guard.ATTESTATION + "\n\n" + f"""Staging URL: https://staging.example.com
Exercised: login
Result: fine
Tested commit: {sha}

- [x] I tested the changes currently on staging
"""
    assert guard.staging_test("main", "staging", pasted, "true", "dijar", sha) == 0

    # Trailing underscores and hyphens are content, not markers.
    underscored = formatted.replace("<https://staging.example.com/app>", "https://staging.example.com/a_b-")
    assert guard.field_value(underscored, "Staging URL") == "https://staging.example.com/a_b-"
    assert guard.staging_test("main", "staging", underscored, "true", "dijar", sha) == 0

    # Labels are matched case-insensitively, for lookup and as a stop line.
    lower = f"""staging url: https://staging.example.com
exercised: login
result:
tested commit: {sha}

- [x] I tested the changes currently on staging
"""
    assert guard.field_value(lower, "Staging URL") == "https://staging.example.com"
    assert not guard.field_present(lower, "Result")
    assert guard.staging_test("main", "staging", lower, "true", "dijar", sha) == 1

    # Multi-line Tested commit is not one SHA.
    two_shas = bulleted.replace(f"Tested commit: {sha}", f"Tested commit:\n- {sha}\n- {'b' * 40}")
    assert guard.staging_test("main", "staging", two_shas, "true", "dijar", sha) == 1


def test_pull_request_fields():
    sha = "a" * 40
    body, found, base, head, author, same = guard.pull_request_fields(
        {
            "body": "hi",
            "base": {"ref": " main ", "repo": {"id": 7}},
            "head": {"sha": f"  {sha}  ", "ref": " staging ", "repo": {"id": 7}},
            "user": {"login": "dependabot[bot]"},
        }
    )
    assert body == "hi"
    assert found == sha
    assert base == "main"
    assert head == "staging"
    assert author == "dependabot[bot]"
    assert same is True
    # A fork: same branch name, different repository.
    *_, same = guard.pull_request_fields(
        {"base": {"ref": "dev", "repo": {"id": 7}}, "head": {"ref": "main", "repo": {"id": 99}}}
    )
    assert same is False
    # Missing ids are not trusted.
    *_, same = guard.pull_request_fields({"base": {"ref": "dev"}, "head": {"ref": "main"}})
    assert same is False
    *_, same = guard.pull_request_fields({"base": {"ref": "dev", "repo": {"id": 7}}, "head": {"ref": "main", "repo": None}})
    assert same is False
    body, found, base, head, author, same = guard.pull_request_fields({"body": None, "head": {}})
    assert (body, found, base, head, author, same) == ("", "", "", "", "", False)
    body, found, base, head, author, same = guard.pull_request_fields({"body": "x", "head": "staging", "user": "x"})
    assert (body, found, base, head, author, same) == ("x", "", "", "", "", False)


class FakeResponse(io.BytesIO):
    def __enter__(self):
        return self

    def __exit__(self, *args):
        return False


def test_retries_transient_errors_only():
    sleeps = []
    attempts = []
    original_open = guard.urllib.request.urlopen
    original_sleep = guard.SLEEP
    guard.SLEEP = sleeps.append

    def flaky(request, timeout=0):
        attempts.append(1)
        if len(attempts) < 3:
            raise urllib.error.HTTPError(request.full_url, 503, "down", {}, None)
        return FakeResponse(json.dumps({"data": {"issues": {"nodes": [{"identifier": "ENG-149"}]}}}).encode())

    def forbidden(request, timeout=0):
        attempts.append(1)
        raise urllib.error.HTTPError(request.full_url, 401, "nope", {}, None)

    def offline(request, timeout=0):
        attempts.append(1)
        raise urllib.error.URLError("offline")

    def quiet(request, timeout=0):
        attempts.append(1)
        raise TimeoutError("read timed out")

    try:
        guard.urllib.request.urlopen = flaky
        assert guard.linear_lookup("secret", "ENG", "149") == "ENG-149"
        assert len(attempts) == 3
        assert sleeps == [1, 2]

        attempts.clear()
        sleeps.clear()
        guard.urllib.request.urlopen = forbidden
        try:
            guard.linear_lookup("secret", "ENG", "149")
            assert False, "401 must fail"
        except guard.LinearError:
            pass
        assert len(attempts) == 1
        assert sleeps == []

        attempts.clear()
        guard.urllib.request.urlopen = offline
        try:
            guard.linear_lookup("secret", "ENG", "149")
            assert False, "network failure must fail closed"
        except guard.LinearError:
            pass
        assert len(attempts) == guard.RETRIES

        attempts.clear()
        guard.urllib.request.urlopen = quiet
        try:
            guard.linear_lookup("secret", "ENG", "149")
            assert False, "read timeout must fail closed after retries"
        except guard.LinearError:
            pass
        assert len(attempts) == guard.RETRIES

        attempts.clear()
        sleeps.clear()
        env = {"GH_TOKEN": "t", "GITHUB_REPOSITORY": "OpacityLabs/sdk", "PR_NUMBER": "901"}
        previous = {key: os.environ.get(key) for key in env}
        os.environ.update(env)
        try:
            guard.urllib.request.urlopen = offline
            try:
                guard.pull_request()
                assert False, "GitHub outage must fail closed"
            except guard.PullRequestLookupError:
                pass
            assert len(attempts) == guard.RETRIES
        finally:
            _restore_env(previous)
    finally:
        guard.urllib.request.urlopen = original_open
        guard.SLEEP = original_sleep


def _restore_env(previous):
    for key, value in previous.items():
        if value is None:
            os.environ.pop(key, None)
        else:
            os.environ[key] = value


ENV_KEYS = ("MODE", "FLOW", "LINEAR_API_KEY", "REQUIRE_STAGING_TEST", "ACTOR")


def test_main_reads_current_pull_request():
    previous = {key: os.environ.get(key) for key in ENV_KEYS}
    original = guard.pull_request
    try:
        os.environ["MODE"] = "guard"
        os.environ["FLOW"] = "three"
        os.environ["LINEAR_API_KEY"] = ""
        guard.pull_request = lambda: ("ship the docs", "", "dev", "nolinear/docs", "dijar", True)
        assert guard.main() == 0
        # Author from the API decides the Dependabot path.
        guard.pull_request = lambda: ("", "", "dev", "dependabot/pip/x-1.0", "dependabot[bot]", True)
        assert guard.main() == 0
        guard.pull_request = lambda: ("", "", "dev", "dependabot/pip/x-1.0", "dijar", True)
        assert guard.main() == 1
        # Back-merge after a hotfix.
        guard.pull_request = lambda: ("", "", "dev", "main", "dijar", True)
        assert guard.main() == 0
        # The same name from a fork is not a back-merge.
        guard.pull_request = lambda: ("", "", "dev", "main", "dijar", False)
        assert guard.main() == 1

        def fail():
            raise guard.PullRequestLookupError("down")

        guard.pull_request = fail
        assert guard.main() == 1
    finally:
        guard.pull_request = original
        _restore_env(previous)


def test_rules_follow_the_live_target():
    previous = {key: os.environ.get(key) for key in ENV_KEYS}
    original = guard.pull_request
    sha = "a" * 40
    attested = f"""Staging URL: https://staging.example.com?preview=123
Exercised: opened the dashboard
Result: the new page loaded
Tested commit: {sha}

- [x] I tested the changes currently on staging
"""
    try:
        os.environ["ACTOR"] = "dijar"
        os.environ["LINEAR_API_KEY"] = ""
        os.environ["MODE"] = "guard"
        os.environ["FLOW"] = "three"
        # Retargeted from dev to main: the live target decides.
        guard.pull_request = lambda: ("ship the docs", "", "main", "nolinear/docs", "dijar", True)
        assert guard.main() == 1
        guard.pull_request = lambda: ("ship the docs", "", "main", "staging", "dijar", True)
        assert guard.main() == 0
        guard.pull_request = lambda: ("ship the docs", "", "", "staging", "dijar", True)
        assert guard.main() == 1

        os.environ["MODE"] = "staging-test"
        os.environ["REQUIRE_STAGING_TEST"] = "true"
        guard.pull_request = lambda: ("", "", "dev", "feature", "dijar", True)
        assert guard.main() == 0
        guard.pull_request = lambda: ("", sha, "main", "staging", "dijar", True)
        assert guard.main() == 1
        guard.pull_request = lambda: (attested, sha, "main", "staging", "dijar", True)
        assert guard.main() == 0
        os.environ["REQUIRE_STAGING_TEST"] = "false"
        guard.pull_request = lambda: ("", "", "main", "staging", "dijar", True)
        assert guard.main() == 0

        def fail():
            raise guard.PullRequestLookupError("down")

        guard.pull_request = fail
        assert guard.main() == 1
    finally:
        guard.pull_request = original
        _restore_env(previous)


def test_staging_rerun_uses_api_sha():
    sha = "a" * 40
    current = "b" * 40
    body = f"""Staging URL: https://staging.example
Exercised: opened the dashboard
Result: the new page loaded
Tested commit: {sha}

- [x] I tested the changes currently on staging
"""
    previous = {key: os.environ.get(key) for key in ENV_KEYS}
    original = guard.pull_request
    try:
        os.environ["MODE"] = "staging-test"
        os.environ["REQUIRE_STAGING_TEST"] = "true"
        os.environ["ACTOR"] = "dijar"
        guard.pull_request = lambda: (body, current, "main", "staging", "dijar", True)
        assert guard.main() == 1
        guard.pull_request = lambda: (body, sha, "main", "staging", "dijar", True)
        assert guard.main() == 0
        guard.pull_request = lambda: (body, "", "main", "staging", "dijar", True)
        assert guard.main() == 1
    finally:
        guard.pull_request = original
        _restore_env(previous)


ALL = [
    test_extract_keys,
    test_linear_branch_match_wins,
    test_first_hop_paths,
    test_linear_error_fails_closed,
    test_flows_do_not_call_linear_on_later_hops,
    test_back_merges_need_no_ticket,
    test_fork_branches_are_not_protected_branches,
    test_dependabot_author_from_api,
    test_staging_attestation,
    test_staging_record_formatting_variants,
    test_pull_request_fields,
    test_retries_transient_errors_only,
    test_main_reads_current_pull_request,
    test_rules_follow_the_live_target,
    test_staging_rerun_uses_api_sha,
]



class BranchFlowGuardTests(unittest.TestCase):
    """One unittest per case so `python3 -m unittest discover -s .github/scripts` reports them."""


for _test in ALL:
    setattr(BranchFlowGuardTests, _test.__name__, staticmethod(_test))


if __name__ == "__main__":
    unittest.main()
