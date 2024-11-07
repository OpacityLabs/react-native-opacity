#ifndef opacity_h
#define opacity_h

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
namespace opacity_core {
#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern const int32_t OPACITY_OK;

extern const int32_t OPACITY_MISSING_POINTER;

extern const int32_t OPACITY_GENERIC_ERROR;

extern const int32_t OPACITY_NOT_SUPPORTED;

int32_t init(const char *api_key_str, bool dry_run);

void execute_workflow(const char *flow);

void emit_webview_event(const char *payload);

bool is_key_stored(const char *key);

const char *start(const char *request);

int32_t get_uber_rider_profile(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_uber_rider_trip_history(const char *cursor,
                                    char **json_ptr,
                                    char **proof_ptr,
                                    char **err_ptr);

int32_t get_uber_rider_trip(const char *trip_uuid,
                            char **json_ptr,
                            char **proof_ptr,
                            char **err_ptr);

int32_t get_uber_driver_profile(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_uber_driver_trips(const char *start_date,
                              const char *end_date,
                              const char *cursor,
                              char **json_ptr,
                              char **proof_ptr,
                              char **err_ptr);

int32_t get_uber_fare_estimate(double pickup_latitude,
                               double pickup_longitude,
                               double destination_latitude,
                               double destination_longitude,
                               char **json_ptr,
                               char **proof_ptr,
                               char **err_ptr);

int32_t get_reddit_account(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_reddit_followed_subreddits(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_reddit_comments(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_reddit_posts(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_zabka_account(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_zabka_points(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_carta_profile(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_carta_organizations(char **json_ptr, char **proof_ptr, char **err_ptr);

int32_t get_carta_portfolio_investments(const char *firm_id,
                                        const char *account_id,
                                        char **json_ptr,
                                        char **proof_ptr,
                                        char **err_ptr);

int32_t get_carta_holdings_companies(const char *account_id,
                                     char **json_ptr,
                                     char **proof_ptr,
                                     char **err_ptr);

int32_t get_carta_corporation_securities(const char *account_id,
                                         const char *corporation_id,
                                         char **json_ptr,
                                         char **proof_ptr,
                                         char **err_ptr);

extern double get_battery_level(void);

extern const char *get_battery_status(void);

extern const char *get_carrier_name(void);

extern const char *get_carrier_mcc(void);

extern const char *get_carrier_mnc(void);

extern double get_course(void);

extern const char *get_cpu_abi(void);

extern double get_altitude(void);

extern double get_latitude(void);

extern double get_longitude(void);

extern const char *get_device_model(void);

extern const char *get_os_name(void);

extern const char *get_os_version(void);

extern bool is_emulator(void);

extern double get_horizontal_accuracy(void);

extern const char *get_ip_address(void);

extern bool is_location_services_enabled(void);

extern double get_vertical_accuracy(void);

extern bool is_wifi_connected(void);

extern bool is_rooted(void);

extern void secure_set(const char *key, const char *value);

extern const char *secure_get(const char *key);

const char *verify(const char *proof);

extern void android_prepare_request(const char *_url);

extern void android_set_request_header(const char *_key, const char *_value);

extern void android_present_webview(void);

extern void android_close_webview(void);

extern void ios_prepare_request(const char *url);

extern void ios_set_request_header(const char *key, const char *value);

extern void ios_present_webview(void);

extern void ios_close_webview(void);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#ifdef __cplusplus
}  // namespace opacity_core
#endif  // __cplusplus

#endif  /* opacity_h */