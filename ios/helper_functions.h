#ifdef __cplusplus
extern "C"
{
#endif

    // Webview functions
    void ios_prepare_request(const char *url);
    void ios_set_request_header(const char *key, const char *value);
    void ios_set_cookie(const char *url, const char *value);
    void ios_present_webview(bool interceptRequests);
    void ios_close_webview();
    const char *ios_get_browser_cookies_for_domain(const char *domain);
    const char *ios_get_browser_cookies_for_current_url();
    void ios_webview_change_url(const char *url);
    const char *ios_eval_js(const char *js, double timeout_in_seconds);

    // Device information functions
    double get_battery_level();
    const char *get_battery_status();
    const char *get_carrier_name();
    const char *get_carrier_mcc();
    const char *get_carrier_mnc();
    double get_course();
    const char *get_cpu_abi();
    double get_altitude();
    double get_latitude();
    double get_longitude();
    const char *get_device_model();
    const char *get_os_name();
    const char *get_os_version();
    const char *get_device_locale();
    bool is_emulator();
    int get_screen_width();
    int get_screen_height();
    float get_screen_density();
    int get_screen_dpi();
    const char *get_device_cpu();
    const char *get_device_codename();
    double get_horizontal_accuracy();
    double get_vertical_accuracy();
    const char *get_ip_address();
    bool is_location_services_enabled();
    bool is_wifi_connected();
    bool is_rooted();
    bool is_app_foregrounded();

#ifdef __cplusplus
}
#endif