#ifndef _THREAD_H
#define _THREAD_H

#include <vector>
#include </home/anna/Downloads/new_folder/bluez-5.66/gdbus/gdbus.h> //<gdbus/gdbus.h>

#include "wx/wx.h"
#include "main.h"

// Thread class that will periodically send events to the GUI thread
class IC2Thread : public wxThread
{
    IC2Frame *m_frame;
    GMainLoop *main_loop;
    int nconnections;
    char *path;
    int quit;

    struct proxies_s {
        GDBusProxy *adapter;
        GDBusProxy *device;

        // org.bluez.Battery1
        GDBusProxy *battery1;

        // Battery Service
        GDBusProxy *battery_service;
        struct battery_service_s {
            GDBusProxy *battery_level;
            GDBusProxy *battery_level_status;
            GDBusProxy *estimated_service_date;
            GDBusProxy *battery_critical_status;
            GDBusProxy *battery_energy_status;
            GDBusProxy *battery_time_status;
            GDBusProxy *battery_health_status;
            GDBusProxy *battery_health_information;
            GDBusProxy *battery_information;
            GDBusProxy *manufacturer_name_string;
            GDBusProxy *model_number_string;
            GDBusProxy *serial_number_string;
        } battery;

        // Device Information Service
        GDBusProxy *device_information_service;
        struct device_information_service_s {
            GDBusProxy *system_id;
            GDBusProxy *model_number_string;
            GDBusProxy *serial_number_string;
            GDBusProxy *firmware_revision_string;
            GDBusProxy *hardware_revision_string;
            GDBusProxy *software_revision_string;
            GDBusProxy *manufacturer_name_string;
            GDBusProxy *ieee_11073_20601_regulatory_certification_data_list;
            GDBusProxy *pnp_id;
        } device_information;

        // Cycling Power Service
        GDBusProxy *cycling_power_service;
        struct cycling_power_service_s {
            GDBusProxy *cycling_power_feature;
            GDBusProxy *cycling_power_measurement;
            GDBusProxy *cycling_power_measurement_broadcast;
            GDBusProxy *sensor_location;
            GDBusProxy *cycling_power_control_point;
            GDBusProxy *cycling_power_vector;
        } cycling_power;

        // Custom Service
        GDBusProxy *custom_service;
        struct custom_service_s {
            GDBusProxy *raw_data;
            GDBusProxy *control_point;
        } custom;
    } proxies;

    struct device {
        GDBusProxy *device;
        char address[20];
    };

    std::vector<struct device *> devices;

    struct write_attribute_data {
        int len;
        void *data;
    };

    enum services {
        DEVICE_INFORMATION,
        BATTERY,
        CYCLING_POWER,
        CUSTOM,
        OTHER,
    } current_service = OTHER;


public:
    IC2Thread(IC2Frame *frame);
    ~IC2Thread();
    virtual ExitCode Entry();

    void print_iter(DBusMessageIter *iter, void **value = NULL, int *length = NULL);
    void print_fixed_iter(DBusMessageIter *iter, void **value = NULL, int *length = NULL);

    static void connect_handler(DBusConnection *connection, void *user_data);
    static void disconnect_handler(DBusConnection *connection, void *user_data);
    static void message_handler(DBusConnection *connection, DBusMessage *message, void *user_data);
    static void proxy_added(GDBusProxy *proxy, void *user_data);
    static void proxy_removed(GDBusProxy *proxy, void *user_data);
    static void property_changed(GDBusProxy *proxy, const char *name, DBusMessageIter *iter, void *user_data);
    static void client_ready(GDBusClient *client, void *user_data);

    static void read_setup(DBusMessageIter *iter, void *user_data);
    static int read_reply(DBusMessage *message, uint8_t **value, int *len, IC2Frame *frame);
    static void read_battery_level(DBusMessage *message, void *user_data);
    static void read_manufacturer_name(DBusMessage *message, void *user_data);
    static void read_model_number(DBusMessage *message, void *user_data);
    static void read_serial_number(DBusMessage *message, void *user_data);
    static void read_hardware_revision(DBusMessage *message, void *user_data);
    static void read_firmware_revision(DBusMessage *message, void *user_data);
    static void read_software_revision(DBusMessage *message, void *user_data);
    static void read_system_id(DBusMessage *message, void *user_data);
    static void read_IEEE(DBusMessage *message, void *user_data);
    static void read_PNP(DBusMessage *message, void *user_data);
    static void read_cycling_power_feature(DBusMessage *message, void *user_data);
    static void read_sensor_location(DBusMessage *message, void *user_data);

    static void write_setup(DBusMessageIter *iter, void *user_data);
    static void write_reply(DBusMessage *message, void *user_data);
    static void notify_reply(DBusMessage *message, void *user_data);


    void adapter_added(GDBusProxy *proxy);
    void device_added(GDBusProxy *proxy);
    void device_removed(GDBusProxy *proxy);
    void service_added(GDBusProxy *proxy);
    void characteristic_added(GDBusProxy *proxy);
    void descriptor_added(GDBusProxy *proxy);

    static void device_connected(DBusMessage *message, void *user_data);
    static void device_disconnected(DBusMessage *message, void *user_data);
    static void free_path(gpointer mem);
    static void remove_device_setup(DBusMessageIter *iter, void *user_data);
    static void device_removed(DBusMessage *message, void *user_data);

    void connect(const char *address);
    void disconnect(const char *address = NULL);

//    template <class input> void set_control_value(uint8_t op_code, input value);

    int concat(uint8_t *cmd);
    template <class T, class... Rest> int concat(uint8_t *cmd, T arg1, Rest...args);
    template <class... Rest> int concat(uint8_t *cmd, const char *arg1, Rest...args);
    template <class ...T> void write_proxy(GDBusProxy *proxy, uint8_t op_code, T ...args);

    static gboolean command_dispatcher(GIOChannel *channel, GIOCondition cond, gpointer data);
};

#endif /* _THREAD_H */
