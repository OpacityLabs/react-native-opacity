"""Change-record check for the branch-flow guard.

The GitHub Linear app only links a branch when the name contains an issue id.
It does not tell Actions whether that issue exists. This script asks Linear.

Runs as MODE=guard (branch flow and Linear issue) or MODE=staging-test (the
staging record on a pull request from staging into main).
"""

import json
import os
import re
import sys
import time
import urllib.error
import urllib.request
from datetime import datetime, timezone
from urllib.parse import urlsplit

LINEAR_URL = "https://api.linear.app/graphql"

# Linear's own branch matching, the same rules its GitHub integration uses to
# link a branch to an issue. Returns the issue or null.
BRANCH_QUERY = """
query($branch: String!) {
  issueVcsBranchSearch(branchName: $branch) { identifier }
}
"""

# Fallback by team key and number, for a branch Linear does not match itself.
KEY_QUERY = """
query($key: String!, $num: Float!) {
  issues(
    includeArchived: true
    filter: { team: { key: { eq: $key } }, number: { eq: $num } }
  ) {
    nodes { identifier }
  }
}
"""

ATTESTATION = """Staging URL:
Exercised:
Result:
Tested commit:

- [ ] I tested the changes currently on staging"""

ACCEPTED_FORM = """Accepted form: each value goes after the label on the same line, or on the
lines directly under it. Bold labels, backticks, and <autolinks> are fine.
Staging URL is an http(s) URL. Tested commit is the 40-character head SHA of
this pull request. Tick the box."""

LABELS = ("Staging URL", "Exercised", "Result", "Tested commit")
CHECKBOX_TEXT = "I tested the changes currently on staging"

# Branch order per flow. A pull request whose head is later in the flow than
# its base is a back-merge (for example main into dev after a hotfix). The
# hotfix pull request already carried the ticket, so the back-merge needs none.
FLOWS = {
    "three": ("dev", "staging", "main"),
    "two": ("staging", "main"),
    "zorya": ("main",),
}

RETRIES = 3
SLEEP = time.sleep


class LinearError(Exception):
    pass


class PullRequestLookupError(Exception):
    pass


class TransientHTTPError(Exception):
    pass


# A key starts on a boundary anywhere in the name, in either case:
# adrianp/ENG-149-title, adrianp/eng-149-title, adrianp/fix-ENG-149-login.
KEY_ON_BOUNDARY = re.compile(r"(?<![A-Za-z0-9])([A-Za-z][A-Za-z0-9]{1,15})-(\d+)")


def extract_keys(head):
    """Return the first Linear key in the branch name, or an empty list.

    change-record-guard gives nothing. release-2024 looks like a key; Linear
    then says it does not exist, which is the honest answer for that name.
    """
    match = KEY_ON_BOUNDARY.search(head)
    if not match:
        return []
    team = match.group(1).upper()
    number = match.group(2)
    return [(team, number, f"{team}-{number}")]


def _post_json(request):
    """One HTTP attempt. 429, 5xx, and connection problems are transient."""
    try:
        with urllib.request.urlopen(request, timeout=20) as response:
            return response.read().decode()
    except urllib.error.HTTPError as error:
        if error.code == 429 or error.code >= 500:
            raise TransientHTTPError(str(error.code)) from error
        raise
    except OSError as error:
        # URLError (DNS, refused) and a read timeout (socket.timeout) are both OSError.
        raise TransientHTTPError("network") from error


def _with_retries(request):
    """Retry transient failures a few times. The gate stays closed on real errors."""
    last = None
    for attempt in range(max(RETRIES, 1)):
        try:
            return _post_json(request)
        except TransientHTTPError as error:
            last = error
            if attempt + 1 < RETRIES:
                SLEEP(2**attempt)
    raise last


def _linear_graphql(api_key, query, variables, what, level="error"):
    """Run one Linear query and return its data. Any failure raises LinearError.

    level is the annotation printed on failure. A lookup that has a fallback
    passes "warning" so a run that still passes does not carry an error.
    """
    payload = json.dumps({"query": query, "variables": variables}).encode()
    request = urllib.request.Request(
        LINEAR_URL,
        data=payload,
        headers={
            "Authorization": api_key,
            "Content-Type": "application/json",
            "Accept": "application/json",
        },
        method="POST",
    )
    try:
        raw = _with_retries(request)
    except urllib.error.HTTPError as error:
        print(f"::{level}::Linear API returned HTTP {error.code} while checking {what}.", file=sys.stderr)
        raise LinearError(str(error.code)) from error
    except TransientHTTPError as error:
        print(
            f"::{level}::Linear API did not answer after {RETRIES} attempts while checking {what} ({error}).",
            file=sys.stderr,
        )
        raise LinearError(str(error)) from error

    try:
        data = json.loads(raw)
    except json.JSONDecodeError as error:
        print(f"::{level}::Linear API returned a response that was not JSON while checking {what}.", file=sys.stderr)
        raise LinearError("json") from error

    errors = data.get("errors") or []
    if errors:
        message = errors[0].get("message") or "unknown error"
        print(f"::{level}::Linear rejected the lookup for {what}: {message}", file=sys.stderr)
        raise LinearError(message)
    return data.get("data") or {}


def linear_branch_lookup(api_key, branch):
    """Ask Linear which issue this branch name belongs to. None when it matches nothing."""
    # The key lookup is the fallback, so a failure here is a warning, not an error.
    data = _linear_graphql(api_key, BRANCH_QUERY, {"branch": branch}, f"branch '{branch}'", level="warning")
    issue = data.get("issueVcsBranchSearch")
    if not isinstance(issue, dict):
        return None
    return issue.get("identifier") or None


def linear_lookup(api_key, team, number):
    """Ask Linear whether TEAM-number exists, archived issues included."""
    data = _linear_graphql(api_key, KEY_QUERY, {"key": team, "num": int(number)}, f"{team}-{number}")
    nodes = (data.get("issues") or {}).get("nodes") or []
    if not nodes:
        return None
    return nodes[0].get("identifier") or f"{team}-{number}"


DEPENDABOT_AUTHORS = ("dependabot[bot]", "dependabot-preview[bot]")


def check_first_hop(base, head, body, api_key, lookup, author="", branch_lookup=None):
    if branch_lookup is None:
        branch_lookup = linear_branch_lookup
    # The branch name is not proof. Anyone can open dependabot/manual-change.
    # The author comes from the same live API response as the branches.
    if re.fullmatch(r"dependabot/.+", head) or re.fullmatch(r"dependabot-preview/.+", head):
        if author in DEPENDABOT_AUTHORS:
            print(f"OK: Dependabot pull request from '{head}' needs no Linear issue.")
            return 0

    lowered = head.lower()
    if lowered == "nolinear" or lowered.startswith("nolinear/"):
        reason = head.split("/", 1)[1] if "/" in head else ""
        if not reason.strip():
            print(
                "::error::Use nolinear/<short-reason>. The text after the slash is the change name.",
                file=sys.stderr,
            )
            return 1
        if not body.strip():
            print(
                "::error::nolinear/ branches are the no-ticket path, so the pull request description is the change record. Describe what this change is and why.",
                file=sys.stderr,
            )
            return 1
        print(f"OK: '{head}' is a no-ticket change and the pull request has a description.")
        return 0

    keys = extract_keys(head)
    if not keys:
        print(
            f"::error::Pull requests into '{base}' must come from a branch that names a real Linear issue (for example adrianp/ENG-149-short-title), from nolinear/<reason> with a pull request description, or from Dependabot. '{head}' does not.",
            file=sys.stderr,
        )
        return 1

    if not api_key.strip():
        print(
            "::error::LINEAR_API_KEY is not available to this repository, so the Linear issue cannot be verified.",
            file=sys.stderr,
        )
        return 1

    # Linear matches the branch name with its own rules first. That is the
    # same match its GitHub integration uses to link this branch to an issue.
    # If that query fails, the key lookup below is the second chance. Both
    # failing still fails closed.
    try:
        identifier = branch_lookup(api_key, head)
    except LinearError:
        identifier = None
    if identifier:
        print(f"OK: Linear links branch '{head}' to issue {identifier}.")
        return 0

    checked = []
    for team, number, key in keys:
        checked.append(key)
        try:
            identifier = lookup(api_key, team, number)
        except LinearError:
            return 1
        if identifier:
            print(f"OK: Linear issue {identifier} exists.")
            return 0

    joined = ", ".join(checked)
    print(
        f"::error::Linear has no issue for branch '{head}' ({joined} does not exist). Create the issue, or rename the branch to nolinear/<reason> and describe the change in the pull request.",
        file=sys.stderr,
    )
    return 1


def exact(base, head, expected, flow_name):
    if head != expected:
        print(
            f"::error::Branch flow violation: PRs into '{base}' must come from '{expected}', but this PR is from '{head}'. The required flow is {flow_name}.",
            file=sys.stderr,
        )
        return 1
    print(f"OK: '{head}' -> '{base}' follows the {flow_name} flow.")
    return 0


def enforce(flow, base, head, body, api_key, lookup=linear_lookup, author="", same_repo=True, branch_lookup=None):
    order = FLOWS.get(flow)
    if order is None:
        print(f"::error::Unknown branch flow '{flow}'.", file=sys.stderr)
        return 1
    flow_name = " -> ".join(order)
    if base not in order:
        if flow == "zorya":
            print(f"::error::Zorya deploys only from main; PR base was '{base}'.", file=sys.stderr)
            return 1
        print(f"No branch-flow rule for base '{base}'; skipping.")
        return 0
    if head in order and not same_repo:
        # A fork can name a branch main or staging. Only this repository's
        # branches are the protected ones, on back-merges and forward hops.
        print(
            f"::error::Branch '{head}' comes from another repository, or GitHub did not return both repository ids, so it is not this repository's '{head}'. Pull requests from forks need a Linear issue in the branch name or nolinear/<reason> with a description.",
            file=sys.stderr,
        )
        return 1
    if head in order and order.index(head) > order.index(base):
        # A back-merge after a hotfix carries no ticket. The hotfix PR had one.
        print(f"OK: '{head}' -> '{base}' is a back-merge and needs no Linear issue.")
        return 0
    position = order.index(base)
    if position == 0:
        return check_first_hop(base, head, body, api_key, lookup, author, branch_lookup)
    return exact(base, head, order[position - 1], flow_name)


def _clean_value(raw):
    value = raw.strip()
    # Closing emphasis and quote markers are formatting. Hyphens and
    # underscores stay: they are legal in URLs and SHAs are unaffected.
    value = re.sub(r"[>*`]+\Z", "", value).strip()
    # Backticks and autolink brackets around the whole value are formatting too.
    value = value.strip("`<>").strip()
    return value


def _label_line(label):
    # Allows **Label:** value, **Label**: value, > Label: value, `Label`: value.
    # Case-insensitive, like OTHER_LABEL, since the template is copied by hand.
    return re.compile(
        rf"^[ \t>*_`-]*{re.escape(label)}[ \t*_`]*:[ \t>*_`-]*(.*?)\s*$",
        re.IGNORECASE,
    )


OTHER_LABEL = re.compile(
    r"^[ \t>*_`-]*(?:" + "|".join(re.escape(label) for label in LABELS) + r")[ \t*_`]*:",
    re.IGNORECASE,
)
CHECKBOX_LINE = re.compile(r"^[ \t]*[-*][ \t]+\[[ xX]\]")


def field_value(body, label):
    """The first non-empty value for a label.

    The value may sit on the label line, or on the following lines up to the
    next blank line, label, or checkbox. That accepts a bulleted list under
    Exercised:. A blank template above a filled copy does not hide the copy.
    """
    lines = body.splitlines()
    pattern = _label_line(label)
    for index, line in enumerate(lines):
        match = pattern.match(line)
        if not match:
            continue
        value = _clean_value(match.group(1))
        if value:
            return value
        collected = []
        for following in lines[index + 1:]:
            if not following.strip():
                break
            if OTHER_LABEL.match(following) or CHECKBOX_LINE.match(following):
                break
            item = _clean_value(re.sub(r"^[ \t>]*[-*+][ \t]+", "", following))
            if item:
                collected.append(item)
        if collected:
            return "\n".join(collected)
    return ""


def field_present(body, label):
    return bool(field_value(body, label))


def staging_url_ok(value):
    """An http(s) URL with a real hostname. Query strings and fragments are fine."""
    value = (value or "").strip()
    if not value or re.search(r"\s", value):
        return False
    try:
        parts = urlsplit(value)
    except ValueError:
        return False
    host = parts.hostname or ""
    return parts.scheme in ("http", "https") and re.search(r"[A-Za-z0-9]", host) is not None


def staging_attestation_required(base, head, required):
    return base == "main" and head == "staging" and required == "true"


def staging_test(base, head, body, required, actor, head_sha=""):
    if not staging_attestation_required(base, head, required):
        print("Staging attestation is not required for this pull request.")
        return 0
    missing = []
    if not staging_url_ok(field_value(body, "Staging URL")):
        missing.append("Staging URL as an http(s) URL")
    for label in ("Exercised", "Result"):
        if not field_present(body, label):
            missing.append(label)
    tested = field_value(body, "Tested commit")
    if re.fullmatch(r"[0-9a-fA-F]{40}", tested) is None:
        missing.append("Tested commit as one 40-character SHA")
    checkbox = re.search(
        rf"(?m)^[ \t]*[-*][ \t]+\[[xX]\][ \t]+{re.escape(CHECKBOX_TEXT)}[ \t]*\.?[ \t]*\r?$",
        body,
    )
    if checkbox is None:
        missing.append("the ticked staging checkbox")
    if missing:
        print(
            "::error::This pull request promotes staging to main. These are missing from the description or are not in the accepted form: "
            + ", ".join(missing)
            + ".",
            file=sys.stderr,
        )
        print(ACCEPTED_FORM)
        print(ATTESTATION)
        return 1
    expected = (head_sha or "").strip().lower()
    if re.fullmatch(r"[0-9a-fA-F]{40}", expected) is None:
        print(
            "::error::GitHub did not return the current pull request head SHA, so the record cannot be tied to a commit.",
            file=sys.stderr,
        )
        return 1
    if tested.lower() != expected:
        print(
            f"::error::Tested commit {tested} is not the current pull request head {expected}. A new commit needs a new staging test. Update Tested commit after you re-test.",
            file=sys.stderr,
        )
        return 1
    now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    print(f"Staging attestation accepted by {actor} at {now} for {expected}.")
    return 0


def pull_request_fields(data):
    """Live description, head SHA, branch names, author, and whether the head
    branch lives in this repository, all from one response."""
    if not isinstance(data, dict):
        raise PullRequestLookupError("json")
    body = data.get("body")
    if not isinstance(body, str):
        body = ""
    head = data.get("head") if isinstance(data.get("head"), dict) else {}
    base = data.get("base") if isinstance(data.get("base"), dict) else {}
    user = data.get("user") if isinstance(data.get("user"), dict) else {}

    def text(source, key):
        raw = source.get(key)
        return raw.strip() if isinstance(raw, str) else ""

    def repo_id(source):
        repo = source.get("repo")
        if not isinstance(repo, dict):
            return None
        value = repo.get("id")
        return value if isinstance(value, int) else None

    head_repo = repo_id(head)
    base_repo = repo_id(base)
    # Missing ids fail closed: a branch is only trusted when both sides are known and equal.
    same_repo = head_repo is not None and head_repo == base_repo

    return body, text(head, "sha"), text(base, "ref"), text(head, "ref"), text(user, "login"), same_repo


def pull_request():
    """Read the pull request from GitHub now. A rerun keeps the old event payload."""
    token = (os.environ.get("GH_TOKEN") or os.environ.get("GITHUB_TOKEN") or "").strip()
    repo = (os.environ.get("GITHUB_REPOSITORY") or "").strip()
    number = (os.environ.get("PR_NUMBER") or "").strip()
    if not token or "/" not in repo or not number.isdigit():
        print(
            "::error::The workflow did not provide the pull request number, repository, or token, so the current pull request cannot be read.",
            file=sys.stderr,
        )
        raise PullRequestLookupError("missing")
    url = f"https://api.github.com/repos/{repo}/pulls/{number}"
    request = urllib.request.Request(
        url,
        headers={
            "Authorization": f"Bearer {token}",
            "Accept": "application/vnd.github+json",
            "X-GitHub-Api-Version": "2022-11-28",
            "User-Agent": "opacity-branch-flow-guard",
        },
    )
    try:
        raw = _with_retries(request)
    except urllib.error.HTTPError as error:
        print(
            f"::error::GitHub returned HTTP {error.code} while reading the pull request.",
            file=sys.stderr,
        )
        raise PullRequestLookupError(str(error.code)) from error
    except TransientHTTPError as error:
        print(
            f"::error::GitHub did not return the pull request after {RETRIES} attempts ({error}).",
            file=sys.stderr,
        )
        raise PullRequestLookupError(str(error)) from error
    try:
        data = json.loads(raw)
    except json.JSONDecodeError as error:
        print(
            "::error::GitHub returned a pull request response that was not JSON.",
            file=sys.stderr,
        )
        raise PullRequestLookupError("json") from error
    return pull_request_fields(data)


def main():
    mode = os.environ.get("MODE", "guard")
    # A rerun keeps the event's base, head, and SHA from the original run.
    # Retargeting the pull request does not update that event, so both jobs
    # read the current branches before choosing a rule.
    try:
        body, head_sha, base, head, author, same_repo = pull_request()
    except PullRequestLookupError:
        return 1
    if not base or not head:
        print(
            "::error::GitHub did not return the current base and head branches, so this run cannot tell which rules apply.",
            file=sys.stderr,
        )
        return 1
    if mode == "staging-test":
        return staging_test(
            base,
            head,
            body,
            os.environ.get("REQUIRE_STAGING_TEST", ""),
            os.environ.get("ACTOR") or os.environ.get("GITHUB_ACTOR") or "",
            head_sha,
        )
    return enforce(
        os.environ.get("FLOW", ""),
        base,
        head,
        body,
        os.environ.get("LINEAR_API_KEY", ""),
        author=author,
        same_repo=same_repo,
    )


if __name__ == "__main__":
    sys.exit(main())
