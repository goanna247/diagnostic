
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <libexplain/bind.h>
#include <tuple>

#include <glib.h>
#include "/home/anna/Downloads/new_folder/bluez-5.66/gdbus/gdbus.h" //gdbus/gdbus.h

#include "uuid.h"
#include "thread.h"

//--------------------------------------------------------------------------------------------------
// DBus print message iter fixed array
//--------------------------------------------------------------------------------------------------
void IC2Thread::print_fixed_iter(DBusMessageIter *iter, void **value, int *length)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    static union {
        dbus_bool_t *valbool;
        dbus_uint32_t *valu32;
        dbus_uint16_t *valu16;
        dbus_int16_t *vals16;
        unsigned char *byte;
    } val;

    int len;

    switch (dbus_message_iter_get_arg_type(iter)) {
    case DBUS_TYPE_BOOLEAN:
        dbus_message_iter_get_fixed_array(iter, &val.valbool, &len);

        for (int i = 0; i < len; i ++) {
            printf("%s ", val.valbool[i] ? "True" : "False");
        }
        break;
    case DBUS_TYPE_UINT32:
        dbus_message_iter_get_fixed_array(iter, &val.valu32, &len);

        for (int i = 0; i < len; i ++) {
            printf("0x%08x ", val.valu32[i]);
        }
        break;
    case DBUS_TYPE_UINT16:
        dbus_message_iter_get_fixed_array(iter, &val.valu16, &len);

        for (int i = 0; i < len; i ++) {
            printf("0x%08x ", val.valu16[i]);
        }
        break;
    case DBUS_TYPE_INT16:
        dbus_message_iter_get_fixed_array(iter, &val.vals16, &len);

        for (int i = 0; i < len; i ++) {
            printf("%d ", val.vals16[i]);
        }
        break;
    case DBUS_TYPE_BYTE:
        dbus_message_iter_get_fixed_array(iter, &val.byte, &len);

        for (int i = 0; i < len; i ++) {
            printf("0x%02x ", val.byte[i]);
        }
        break;
    default:
        break;
    };

    // Send the data back to the caller
    if (length) {
        *length = len;
    }
    if (value) {
        *value = val.byte;
    }
}

//--------------------------------------------------------------------------------------------------
// DBus print message iter
//--------------------------------------------------------------------------------------------------
void IC2Thread::print_iter(DBusMessageIter *iter, void **value, int *length)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    static union {
        dbus_bool_t valbool[1];
        dbus_uint32_t valu32[1];
        dbus_uint16_t valu16[1];
        dbus_int16_t vals16[1];
        unsigned char byte[1];
        const char *valstr;
    } val;

    // Send the data back to the caller
    if (length) {
        *length = 1;
    }
    if (value) {
        *value = val.byte;
    }

    DBusMessageIter subiter;
    char *entry;

    if (iter == NULL) {
        printf("NULL");
        return;
    }

    switch (dbus_message_iter_get_arg_type(iter)) {
    case DBUS_TYPE_INVALID:
        printf("DBUS_TYPE_INVALID");
        break;
    case DBUS_TYPE_STRING:
    case DBUS_TYPE_OBJECT_PATH:
        dbus_message_iter_get_basic(iter, &val.valstr);
        printf("%s", val.valstr);
        break;
    case DBUS_TYPE_BOOLEAN:
        dbus_message_iter_get_basic(iter, val.valbool);
        printf("%s", val.valbool[0] == TRUE ? "true" : "false");
        break;
    case DBUS_TYPE_UINT32:
        dbus_message_iter_get_basic(iter, val.valu32);
        printf("0x%08x", val.valu32[0]);
        break;
    case DBUS_TYPE_UINT16:
        dbus_message_iter_get_basic(iter, val.valu16);
        printf("0x%04x", val.valu16[0]);
        break;
    case DBUS_TYPE_INT16:
        dbus_message_iter_get_basic(iter, val.vals16);
        printf("%d", val.vals16[0]);
        break;
    case DBUS_TYPE_BYTE:
        dbus_message_iter_get_basic(iter, val.byte);
        printf("0x%02x (%d)", val.byte[0], val.byte[0]);
        break;
    case DBUS_TYPE_VARIANT:
        dbus_message_iter_recurse(iter, &subiter);
        print_iter(&subiter, value, length);
        break;
    case DBUS_TYPE_ARRAY:
        dbus_message_iter_recurse(iter, &subiter);

        if (dbus_type_is_fixed(dbus_message_iter_get_arg_type(&subiter))) {
            print_fixed_iter(&subiter, value, length);
            break;
        }

        while (dbus_message_iter_get_arg_type(&subiter) != DBUS_TYPE_INVALID) {
            print_iter(&subiter, value, length);
            printf(" ");
            dbus_message_iter_next(&subiter);
        }
        break;
    case DBUS_TYPE_DICT_ENTRY:
        dbus_message_iter_recurse(iter, &subiter);
        printf("Key: ");
        print_iter(&subiter, value, length);

        dbus_message_iter_next(&subiter);
        printf(" Value: ");
        print_iter(&subiter, value, length);
        break;
    default:
        printf("Unsupported type");
        break;
    }
}

//--------------------------------------------------------------------------------------------------
// DBus connect handler
//--------------------------------------------------------------------------------------------------
void IC2Thread::connect_handler(DBusConnection *connection, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    puts("DBus connected");
}

//--------------------------------------------------------------------------------------------------
// DBus disconnect handler
//--------------------------------------------------------------------------------------------------
void IC2Thread::disconnect_handler(DBusConnection *connection, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    puts("DBus disconnected");
//     g_free ( proxies );
//     proxies = NULL;
}

//--------------------------------------------------------------------------------------------------
// DBus message handler
//--------------------------------------------------------------------------------------------------
void IC2Thread::message_handler(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    printf("DBus message: %s.%s\n", dbus_message_get_interface(message), dbus_message_get_member(message));
}

//--------------------------------------------------------------------------------------------------
// DBus proxy added
//--------------------------------------------------------------------------------------------------
void IC2Thread::proxy_added(GDBusProxy *proxy, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    IC2Thread *thread = (IC2Thread *) user_data;
    const char *interface;
    interface = g_dbus_proxy_get_interface(proxy);
    printf("DBus proxy added: %s\n", interface);

    if (!strcmp(interface, "org.bluez.Adapter1")) {
        thread->adapter_added(proxy);
    } else if (!strcmp(interface, "org.bluez.Device1")) {
        thread->device_added(proxy);
    } else if (!strcmp(interface, "org.bluez.GattService1")) {
        thread->service_added(proxy);
    } else if (!strcmp(interface, "org.bluez.GattCharacteristic1")) {
        thread->characteristic_added(proxy);
    } else if (!strcmp(interface, "org.bluez.GattDescriptor1")) {
        thread->descriptor_added(proxy);
    } else if (!strcmp(interface, "org.bluez.Battery1")) {
        thread->proxies.battery1 = proxy;
    }
}

//--------------------------------------------------------------------------------------------------
// DBus proxy removed
//--------------------------------------------------------------------------------------------------
void IC2Thread::proxy_removed(GDBusProxy *proxy, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    IC2Thread *thread = (IC2Thread *) user_data;
    const char *interface;
    interface = g_dbus_proxy_get_interface(proxy);
    printf("DBus proxy removed: %s\n", interface);

    if (!strcmp(interface, "org.bluez.Device1")) {
        thread->device_removed(proxy);
    }
}

//--------------------------------------------------------------------------------------------------
// DBus property changed
//--------------------------------------------------------------------------------------------------
void IC2Thread::property_changed(GDBusProxy *proxy, const char *name, DBusMessageIter *iter, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int length;

    IC2Thread *thread = (IC2Thread *) user_data;
    printf("DBus property changed %s: ", name);
    if (!strcmp(name, "Value")) {
        thread->print_iter(iter, (void **) &value, &length);
        if (proxy == (thread->proxies.cycling_power.cycling_power_control_point)) {
            printf(" Control point");
            thread->m_frame->SetCyclingPowerControlPoint(value, length);
        } else if (proxy == (thread->proxies.battery1)) {
            printf(" Battery level %hhu%%", *value);
            thread->m_frame->SetBatteryLevel(*value);
        } else if (proxy == (thread->proxies.cycling_power.cycling_power_measurement)) {
            printf(" Cycling power measurement");
            thread->m_frame->SetCyclingPowerMeasurement(value, length);
        } else if (proxy == (thread->proxies.custom.control_point)) {
            printf(" InfoCrank control point");
            thread->m_frame->SetInfoCrankControlPoint(value, length);
        } else if (proxy == (thread->proxies.cycling_power.cycling_power_vector)) {
            printf(" Cycling power vector");
            thread->m_frame->SetCyclingPowerVector(value, length);
        } else if (proxy == (thread->proxies.custom.raw_data)) {
            printf(" Raw data");
            thread->m_frame->SetInfoCrankRawData(value, length);
        }
    }
    if (!strcmp(name, "Percentage")) {
        thread->print_iter(iter, (void **) &value, &length);
        if (proxy == (thread->proxies.battery1)) {
            printf(" Battery level %hhu%%", *value);
            thread->m_frame->SetBatteryLevel(*value);
        }
    }
    printf("\n");
}

//--------------------------------------------------------------------------------------------------
// DBus client ready
//--------------------------------------------------------------------------------------------------
void IC2Thread::client_ready(GDBusClient *client, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    puts("DBus client ready");
}

//--------------------------------------------------------------------------------------------------
// DBus read setup
//--------------------------------------------------------------------------------------------------
void IC2Thread::read_setup(DBusMessageIter *iter, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    DBusMessageIter dict;
    uint16_t offset = 0;

    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
                                     DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_STRING_AS_STRING
                                     DBUS_TYPE_VARIANT_AS_STRING
                                     DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
                                     &dict);

    g_dbus_dict_append_entry(&dict, "offset", DBUS_TYPE_UINT16, &offset);

    dbus_message_iter_close_container(iter, &dict);
}

//--------------------------------------------------------------------------------------------------
// DBus read reply
//--------------------------------------------------------------------------------------------------
void IC2Thread::read_battery_level(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetBatteryLevel(*value);
    }
}

void IC2Thread::read_manufacturer_name(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetManufacturerName((char *) value);
    }
}

void IC2Thread::read_model_number(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetModelNumber((char *) value);
    }
}

void IC2Thread::read_serial_number(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetSerialNumber((char *) value);
    }
}

void IC2Thread::read_hardware_revision(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetHardwareRevisionNumber((char *) value);
    }
}

void IC2Thread::read_firmware_revision(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetFirmwareRevisionNumber((char *) value);
    }
}

void IC2Thread::read_software_revision(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetSoftwareRevisionNumber((char *) value);
    }
}

void IC2Thread::read_system_id(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);

    DBusError error;
    dbus_error_init(&error);
    if (dbus_set_error_from_message(&error, message) == TRUE) {
        printf("Failed to read system id: %s\n", error.name);
        dbus_error_free(&error);
        return;
    }

    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetSystemID((char *) value);
    }
}

void IC2Thread::read_IEEE(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetIEEE((char *) value);
    }
}

void IC2Thread::read_PNP(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetPNP((char *) value);
    }
}

void IC2Thread::read_cycling_power_feature(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetCyclingPowerFeature((char *) value);
    }
}

void IC2Thread::read_sensor_location(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t *value;
    int len;
    if (!read_reply(message, &value, &len, (IC2Frame *) user_data)) {
        ((IC2Frame *) user_data)->SetSensorLocation(*value);
    }
}

int IC2Thread::read_reply(DBusMessage *message, uint8_t **value, int *len, IC2Frame *frame)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    DBusError error;
    DBusMessageIter iter, array;

    dbus_error_init(&error);

    if (dbus_set_error_from_message(&error, message) == TRUE) {
        printf("Failed to read: %s\n", error.name);
        dbus_error_free(&error);
        frame->SetStatusText("Failed to read", 1);
        return (-1);
    }

    dbus_message_iter_init(message, &iter);

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
        printf("Invalid response to read\n");
        frame->SetStatusText("Invalid response to read", 1);
        return (-2);
    }

    dbus_message_iter_recurse(&iter, &array);
    dbus_message_iter_get_fixed_array(&array, value, len);

    if (*len < 0) {
        printf("Unable to parse value\n");
        frame->SetStatusText("Unable to parse value", 1);
        return (-3);
    }

    //printf ( "%d %s\n", len, value );

    return (0);
}


//--------------------------------------------------------------------------------------------------
// DBus write setup
//--------------------------------------------------------------------------------------------------
void IC2Thread::write_setup(DBusMessageIter *iter, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    struct write_attribute_data *wd = (struct write_attribute_data *) user_data;
    DBusMessageIter array, dict;

    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "y", &array);
    dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE, &wd->data, wd->len);
    dbus_message_iter_close_container(iter, &array);

    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
                                     DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_STRING_AS_STRING
                                     DBUS_TYPE_VARIANT_AS_STRING
                                     DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
                                     &dict);
    dbus_message_iter_close_container(iter, &dict);
}


//--------------------------------------------------------------------------------------------------
// DBus write reply
//--------------------------------------------------------------------------------------------------
void IC2Thread::write_reply(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    DBusError error;

    dbus_error_init(&error);

    if (dbus_set_error_from_message(&error, message) == TRUE) {
        printf("Failed to write: %s\n", error.name);
        dbus_error_free(&error);
        return;
    }
    printf("Write OK\n");
}


//--------------------------------------------------------------------------------------------------
// DBus notify reply
//--------------------------------------------------------------------------------------------------
void IC2Thread::notify_reply(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    DBusError error;

    dbus_error_init(&error);

    if (dbus_set_error_from_message(&error, message) == TRUE) {
        printf("Failed to notify: %s\n", error.name);
        dbus_error_free(&error);
        return;
    }
    printf("Notify OK\n");
}

//--------------------------------------------------------------------------------------------------
// DBus adapter added
//--------------------------------------------------------------------------------------------------
void IC2Thread::adapter_added(GDBusProxy *proxy)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // Use the first adapter
    if (!proxies.adapter) {
        proxies.adapter = proxy;

        // Start scanning for devices
        if (g_dbus_proxy_method_call(proxy, "StartDiscovery", NULL, NULL, NULL, NULL) == FALSE) {
            printf("Failed to start discovery\n");
        }
    }

    // Print adapter information
    DBusMessageIter iter;

    printf("\tAddress: ");
    if (g_dbus_proxy_get_property(proxy, "Address", &iter)) {
        print_iter(&iter);
    }

    printf("\n\tName: ");
    if (g_dbus_proxy_get_property(proxy, "Alias", &iter)) {
        print_iter(&iter);
    }
    printf("\n");
}



//--------------------------------------------------------------------------------------------------
// DBus device added
//--------------------------------------------------------------------------------------------------
void IC2Thread::device_added(GDBusProxy *proxy)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // Print device information
    DBusMessageIter iter;

    printf("\tAddress: ");
    const char *address;
    if (g_dbus_proxy_get_property(proxy, "Address", &iter)) {
        dbus_message_iter_get_basic(&iter, &address);
        print_iter(&iter);
    }

    printf("\n\tAddress type: ");
    if (g_dbus_proxy_get_property(proxy, "AddressType", &iter)) {
        print_iter(&iter);
    }

    printf("\n\tConnected: ");
    if (g_dbus_proxy_get_property(proxy, "Connected", &iter)) {
        print_iter(&iter);
    }

    printf("\n\tName: ");
    const char *name;
    if (g_dbus_proxy_get_property(proxy, "Alias", &iter)) {
        dbus_message_iter_get_basic(&iter, &name);
        print_iter(&iter);
    }

    printf("\n\tUUIDs: ");
    if (g_dbus_proxy_get_property(proxy, "UUIDs", &iter)) {
        print_iter(&iter);

        // Send devices containing a Cycling Power Service to the user interface
        DBusMessageIter subiter;
        dbus_message_iter_recurse(&iter, &subiter);
        while (dbus_message_iter_get_arg_type(&subiter) != DBUS_TYPE_INVALID) {
            const char *uuid;
            dbus_message_iter_get_basic(&subiter, &uuid);
            if (!strcmp(uuid, CYCLING_POWER_SERVICE_UUID)) {

                struct device *device = new (struct device);
                device->device = proxy;
                strcpy(device->address, address);
                devices.push_back(device);
                m_frame->AddDevice(name, address);

            }
            dbus_message_iter_next(&subiter);
        }
    }
    printf("\n");
}

//--------------------------------------------------------------------------------------------------
// DBus device removed
//--------------------------------------------------------------------------------------------------
void IC2Thread::device_removed(GDBusProxy *proxy)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    for (std::vector<struct device *>::iterator it = devices.begin(); it < devices.end(); ++it) {
        if ((*it)->device == proxy) {
            printf("found one to remove\n");
            m_frame->RemoveDevice((*it)->address);
            delete *it;
            devices.erase(it);
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
// DBus service added
//--------------------------------------------------------------------------------------------------
void IC2Thread::service_added(GDBusProxy *proxy)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // Print service information
    DBusMessageIter iter;
    const char *uuid;

    if (g_dbus_proxy_get_property(proxy, "UUID", &iter)) {
        dbus_message_iter_get_basic(&iter, &uuid);

        if (!strcmp(uuid, BATTERY_SERVICE_UUID)) {
            proxies.battery_service = proxy;
            current_service =   BATTERY;
        } else if (!strcmp(uuid, DEVICE_INFORMATION_SERVICE_UUID)) {
            proxies.device_information_service = proxy;
            current_service = DEVICE_INFORMATION;
        } else if (!strcmp(uuid, CYCLING_POWER_SERVICE_UUID)) {
            proxies.cycling_power_service = proxy;
            current_service = CYCLING_POWER;
        } else if (!strcmp(uuid, CUSTOM_SERVICE_UUID)) {
            proxies.custom_service = proxy;
            current_service = CUSTOM;
        } else {
            current_service = OTHER;
        }
    }

    printf("\tPATH: %s", g_dbus_proxy_get_path(proxy));

    printf("\n\tUUID: ");
    if (g_dbus_proxy_get_property(proxy, "UUID", &iter)) {
        print_iter(&iter);
    }

    printf("\n\tPrimary: ");
    if (g_dbus_proxy_get_property(proxy, "Primary", &iter)) {
        print_iter(&iter);
    }
    printf("\n");
}

//--------------------------------------------------------------------------------------------------
// DBus characteristic added
//--------------------------------------------------------------------------------------------------
void IC2Thread::characteristic_added(GDBusProxy *proxy)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    DBusMessageIter iter;
    const char *uuid;
    if (g_dbus_proxy_get_property(proxy, "UUID", &iter)) {
        dbus_message_iter_get_basic(&iter, &uuid);

        switch (current_service) {
        case BATTERY:
            // NOTE  The battery characteristic is not discovered. Instead DBUS uses Battery1
            if (!strcmp(uuid, BATTERY_LEVEL_CHARACTERISTIC_UUID)) {
                proxies.battery.battery_level = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_battery_level, m_frame, NULL);
            } else if (!strcmp(uuid, BATTERY_LEVEL_STATUS_CHARACTERISTIC_UUID)) {
                proxies.battery.battery_level_status = proxy;
            } else if (!strcmp(uuid, ESTIMATED_SERVICE_DATE_CHARACTERISTIC_UUID)) {
                proxies.battery.estimated_service_date = proxy;
            } else if (!strcmp(uuid, BATTERY_CRITICAL_STATUS_CHARACTERISTIC_UUID)) {
                proxies.battery.battery_critical_status = proxy;
            } else if (!strcmp(uuid, BATTERY_ENERGY_STATUS_CHARACTERISTIC_UUID)) {
                proxies.battery.battery_energy_status = proxy;
            } else if (!strcmp(uuid, BATTERY_TIME_STATUS_CHARACTERISTIC_UUID)) {
                proxies.battery.battery_time_status = proxy;
            } else if (!strcmp(uuid, BATTERY_HEALTH_STATUS_CHARACTERISTIC_UUID)) {
                proxies.battery.battery_health_status = proxy;
            } else if (!strcmp(uuid, BATTERY_HEALTH_INFORMATION_CHARACTERISTIC_UUID)) {
                proxies.battery.battery_health_information = proxy;
            } else if (!strcmp(uuid, BATTERY_INFORMATION_CHARACTERISTIC_UUID)) {
                proxies.battery.battery_information = proxy;
            } else if (!strcmp(uuid, MANUFACTURER_NAME_STRING_CHARACTERISTIC_UUID)) {
                proxies.battery.manufacturer_name_string = proxy;
            } else if (!strcmp(uuid, MODEL_NUMBER_STRING_CHARACTERISTIC_UUID)) {
                proxies.battery.model_number_string = proxy;
            } else if (!strcmp(uuid, SERIAL_NUMBER_STRING_CHARACTERISTIC_UUID)) {
                proxies.battery.serial_number_string = proxy;
            }
            break;
        case DEVICE_INFORMATION:
            if (!strcmp(uuid, SYSTEM_ID_CHARACTERISTIC_UUID)) {
                proxies.device_information.system_id = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_system_id, m_frame, NULL);
            } else if (!strcmp(uuid, FIRMWARE_REVISION_STRING_CHARACTERISTIC_UUID)) {
                proxies.device_information.firmware_revision_string = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_firmware_revision, m_frame, NULL);
            } else if (!strcmp(uuid, HARDWARE_REVISION_STRING_CHARACTERISTIC_UUID)) {
                proxies.device_information.hardware_revision_string = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_hardware_revision, m_frame, NULL);
            } else if (!strcmp(uuid, SOFTWARE_REVISION_STRING_CHARACTERISTIC_UUID)) {
                proxies.device_information.software_revision_string = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_software_revision, m_frame, NULL);
            } else if (!strcmp(uuid, IEEE_11073_20601_REGULATORY_CERTIFICATION_DATA_LIST_CHARACTERISTIC_UUID)) {
                proxies.device_information.ieee_11073_20601_regulatory_certification_data_list = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_IEEE, m_frame, NULL);
            } else if (!strcmp(uuid, PNP_ID_CHARACTERISTIC_UUID)) {
                proxies.device_information.pnp_id = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_PNP, m_frame, NULL);
            } else if (!strcmp(uuid, MANUFACTURER_NAME_STRING_CHARACTERISTIC_UUID)) {
                proxies.device_information.manufacturer_name_string = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_manufacturer_name, m_frame, NULL);
            } else if (!strcmp(uuid, MODEL_NUMBER_STRING_CHARACTERISTIC_UUID)) {
                proxies.device_information.model_number_string = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_model_number, m_frame, NULL);
            } else if (!strcmp(uuid, SERIAL_NUMBER_STRING_CHARACTERISTIC_UUID)) {
                proxies.device_information.serial_number_string = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_serial_number, m_frame, NULL);
            }
            break;
        case CYCLING_POWER:
            if (!strcmp(uuid, CYCLING_POWER_FEATURE_CHARACTERISTIC_UUID)) {
                proxies.cycling_power.cycling_power_feature = proxy;
                g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_cycling_power_feature, m_frame, NULL);
            } else if (!strcmp(uuid, CYCLING_POWER_MEASUREMENT_CHARACTERISTIC_UUID)) {
                proxies.cycling_power.cycling_power_measurement = proxy;
            } else if (!strcmp(uuid, SENSOR_LOCATION_CHARACTERISTIC_UUID)) {
                proxies.cycling_power.sensor_location = proxy;
            } else if (!strcmp(uuid, CYCLING_POWER_CONTROL_POINT_CHARACTERISTIC_UUID)) {
                proxies.cycling_power.cycling_power_control_point = proxy;
            } else if (!strcmp(uuid, CYCLING_POWER_VECTOR_CHARACTERISTIC_UUID)) {
                proxies.cycling_power.cycling_power_vector = proxy;
            }
            break;
        case CUSTOM:
            if (!strcmp(uuid, CUSTOM_RAW_DATA_CHARACTERISTIC_UUID)) {
                proxies.custom.raw_data = proxy;
            } else if (!strcmp(uuid, CUSTOM_CONTROL_POINT_CHARACTERISTIC_UUID)) {
                proxies.custom.control_point = proxy;
            }
            break;
        default:
            break;
        }
    }


    // Print characteristic information
    printf("\tPATH: %s", g_dbus_proxy_get_path(proxy));

    printf("\n\tUUID: ");
    if (g_dbus_proxy_get_property(proxy, "UUID", &iter)) {
        print_iter(&iter);
    }

    printf("\n\tFLAGS: ");
    if (g_dbus_proxy_get_property(proxy, "Flags", &iter)) {
        print_iter(&iter);
    }
    printf("\n");
}

//--------------------------------------------------------------------------------------------------
// DBus descriptor added
//--------------------------------------------------------------------------------------------------
void IC2Thread::descriptor_added(GDBusProxy *proxy)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    DBusMessageIter iter;
    const char *uuid;

    if (g_dbus_proxy_get_property(proxy, "UUID", &iter)) {
        dbus_message_iter_get_basic(&iter, &uuid);

        switch (current_service) {
        case BATTERY:
            break;
        case DEVICE_INFORMATION:
            break;
        case CYCLING_POWER:
            if (!strcmp(uuid, SERVER_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR_UUID)) {
                proxies.cycling_power.cycling_power_measurement_broadcast = proxy;
            }
            break;
        case CUSTOM:
            break;
        default:
            break;
        }
    }

    // Print descriptor information
    printf("\tPATH: %s", g_dbus_proxy_get_path(proxy));

    printf("\n\tUUID: ");
    if (g_dbus_proxy_get_property(proxy, "UUID", &iter)) {
        print_iter(&iter);
    }

    printf("\n\tFLAGS: ");
    if (g_dbus_proxy_get_property(proxy, "Flags", &iter)) {
        print_iter(&iter);
    }
    printf("\n");
}

//--------------------------------------------------------------------------------------------------
// Device connected
//--------------------------------------------------------------------------------------------------
void IC2Thread::device_connected(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);

    DBusError error;
    dbus_error_init(&error);

    if (dbus_set_error_from_message(&error, message) == TRUE) {
        printf("Failed to connect: %s\n", error.name);
        dbus_error_free(&error);
        return;
    }

    IC2Thread *thread = (IC2Thread *) user_data;
    thread->nconnections ++;
    printf("Device connected %d\n", thread->nconnections);
}

//--------------------------------------------------------------------------------------------------
// Device disconnected
//--------------------------------------------------------------------------------------------------
void IC2Thread::device_disconnected(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);

    DBusError error;
    dbus_error_init(&error);

    if (dbus_set_error_from_message(&error, message) == TRUE) {
        printf("Failed to disconnect: %s\n", error.name);
        dbus_error_free(&error);
        return;
    }

    IC2Thread *thread = (IC2Thread *) user_data;
    thread->nconnections--;
    printf("Device disconnected %d\n", thread->nconnections);

    if (thread->quit && !thread->nconnections) {
        g_main_loop_quit(thread->main_loop);
    }
}

//--------------------------------------------------------------------------------------------------
// Remove device setup
//--------------------------------------------------------------------------------------------------
void IC2Thread::remove_device_setup(DBusMessageIter *iter, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    //const char *path = (const char *) user_data;
    IC2Thread *thread = (IC2Thread *) user_data;
    dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &(thread->path) /*&path*/);
}

//--------------------------------------------------------------------------------------------------
// Device removed
//--------------------------------------------------------------------------------------------------
void IC2Thread::device_removed(DBusMessage *message, void *user_data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    DBusError error;

    dbus_error_init(&error);

    if (dbus_set_error_from_message(&error, message) == TRUE) {
        printf("Failed to remove device: %s\n", error.name);
        dbus_error_free(&error);
        return;
    }

    IC2Thread *thread = (IC2Thread *) user_data;
    thread->nconnections--;
    printf("Device has been removed %d\n", thread->nconnections);

    if (thread->quit && !thread->nconnections) {
        g_main_loop_quit(thread->main_loop);
    }
}

//--------------------------------------------------------------------------------------------------
// Connect to device
//--------------------------------------------------------------------------------------------------
void IC2Thread::connect(const char *address)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);

    // Search through list of devices on matching address
    for (std::vector<struct device *>::iterator it = devices.begin(); it < devices.end(); ++it) {
        if (!strncmp((*it)->address, address, 17)) {
            printf("found one to connect\n");
            proxies.device = (*it)->device;
            g_dbus_proxy_method_call((*it)->device, "Connect", NULL, device_connected, this, NULL);
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
// Free path member in IC2Thread
//--------------------------------------------------------------------------------------------------
void IC2Thread::free_path(gpointer mem)
{
    IC2Thread *thread = (IC2Thread *) mem;
    g_free(thread->path);
}

//--------------------------------------------------------------------------------------------------
// Disconnect device
//--------------------------------------------------------------------------------------------------
void IC2Thread::disconnect(const char *address)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);

    int nconnected = 0;
    //char *path;
    // Search through list of devices on matching address
    for (std::vector<struct device *>::iterator it = devices.begin(); it < devices.end(); ++it) {
        // Skip this one if address is supplied but does not match
        if (address) {
            if (strncmp((*it)->address, address, 17)) {
                continue;
            }
        }
        // If address is not supplied, disconnect all connected devices
        DBusMessageIter iter;
        if (g_dbus_proxy_get_property((*it)->device, "Connected", &iter)) {
            dbus_bool_t connected;
            dbus_message_iter_get_basic(&iter, &connected);
            if (connected) {
                nconnected ++;
                //printf("About to disconnect %s\n", (*it)->address);
                //g_dbus_proxy_method_call((*it)->device, "Disconnect", NULL, device_disconnected, this, NULL);
                printf("About to remove %s\n", (*it)->address);
                path = g_strdup(g_dbus_proxy_get_path((*it)->device));
                if (!g_dbus_proxy_method_call(proxies.adapter, "RemoveDevice", remove_device_setup, device_removed, this /*path*/, free_path)) {
                    g_free(path);
                }
            }
        }
    }
    proxies.device = NULL;
    if (!nconnected && quit) {
        g_main_loop_quit(main_loop);
    }
}


//--------------------------------------------------------------------------------------------------
// Helper function for concatenating commands for control point write
//--------------------------------------------------------------------------------------------------
int IC2Thread::concat(uint8_t *cmd)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    return 0;
}

template <class T, class... Rest> int IC2Thread::concat(uint8_t *cmd, T arg1, Rest...args)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    memcpy(cmd, &arg1, sizeof(T));
    return sizeof(T) + concat(&cmd[sizeof(T)], args...);
}

template <class... Rest> int IC2Thread::concat(uint8_t *cmd, const char *arg1, Rest...args)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    memcpy(cmd, arg1, strlen(arg1));
    return strlen(arg1) + concat(&cmd[strlen(arg1)], args...);
}


//--------------------------------------------------------------------------------------------------
// Cycling power control point - write
//--------------------------------------------------------------------------------------------------
template <class ...T> void IC2Thread::write_proxy(GDBusProxy *proxy, uint8_t op_code, T ...args)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    uint8_t cmd[32] = {op_code};
    int len = 1;
    len += concat(&cmd[1], args...);
    struct write_attribute_data write_attribute_data = { .len = len, .data = cmd};
    g_dbus_proxy_method_call(proxy, "StartNotify", NULL, notify_reply, NULL, NULL);
    g_dbus_proxy_method_call(proxy, "WriteValue", write_setup, write_reply, &write_attribute_data, NULL);
}



//--------------------------------------------------------------------------------------------------
// Dispatch commands received from the user interface
//--------------------------------------------------------------------------------------------------
gboolean IC2Thread::command_dispatcher(GIOChannel *channel, GIOCondition cond, gpointer data)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    IC2Thread *thread = (IC2Thread *) data;
    gchar *command;
    gsize length, terminator_pos;
    GError *error = NULL;

    g_io_channel_read_line(channel, &command, &length, &terminator_pos, &error);
    if (error) {
        return FALSE;
    }
    command[terminator_pos] = 0x00;               // Replace newline terminator with null terminator

    printf("Command received: %lu %s\n", length, command);

    if (!strcmp(command, "Quit")) {
        thread->quit = true;
        thread->disconnect();
    } else if (!strncmp(command, "Disconnect all", 14)) {
        thread->disconnect();
    } else if (!strncmp(command, "Connect", 7)) {
        thread->connect(&command[8]);
    } else if (!strncmp(command, "Disconnect", 10)) {
        thread->disconnect(&command[11]);
    } else if (!strncmp(command, "Refresh device information", 26)) {
        g_dbus_proxy_method_call(thread->proxies.device_information.system_id, "ReadValue", read_setup, read_system_id, thread->m_frame, NULL);
        g_dbus_proxy_method_call(thread->proxies.device_information.firmware_revision_string, "ReadValue", read_setup, read_firmware_revision, thread->m_frame, NULL);
        g_dbus_proxy_method_call(thread->proxies.device_information.hardware_revision_string, "ReadValue", read_setup, read_hardware_revision, thread->m_frame, NULL);
        g_dbus_proxy_method_call(thread->proxies.device_information.software_revision_string, "ReadValue", read_setup, read_software_revision, thread->m_frame, NULL);
        g_dbus_proxy_method_call(thread->proxies.device_information.ieee_11073_20601_regulatory_certification_data_list, "ReadValue", read_setup, read_IEEE, thread->m_frame, NULL);
        g_dbus_proxy_method_call(thread->proxies.device_information.pnp_id, "ReadValue", read_setup, read_PNP, thread->m_frame, NULL);
        g_dbus_proxy_method_call(thread->proxies.device_information.manufacturer_name_string, "ReadValue", read_setup, read_manufacturer_name, thread->m_frame, NULL);
        g_dbus_proxy_method_call(thread->proxies.device_information.model_number_string, "ReadValue", read_setup, read_model_number, thread->m_frame, NULL);
        g_dbus_proxy_method_call(thread->proxies.device_information.serial_number_string, "ReadValue", read_setup, read_serial_number, thread->m_frame, NULL);
    } else if (!strncmp(command, "Refresh battery information", 27)) {
        g_dbus_proxy_method_call(thread->proxies.battery.battery_level, "ReadValue", read_setup, read_battery_level, thread->m_frame, NULL);
        // TODO read characteristics
    } else if (!strncmp(command, "Refresh features", 16)) {
        g_dbus_proxy_method_call(thread->proxies.cycling_power.cycling_power_feature, "ReadValue", read_setup, read_cycling_power_feature, thread->m_frame, NULL);
    } else if (!strncmp(command, "Notify measurement on", 21)) {
        g_dbus_proxy_method_call(thread->proxies.cycling_power.cycling_power_measurement, "StartNotify", NULL, notify_reply, NULL, NULL);
    } else if (!strncmp(command, "Notify measurement off", 22)) {
        g_dbus_proxy_method_call(thread->proxies.cycling_power.cycling_power_measurement, "StopNotify", NULL, NULL, NULL, NULL);
    } else if (!strncmp(command, "Broadcast measurement on", 24)) {
        uint8_t cmd[2] = {0x01, 0x00};
        struct write_attribute_data write_attribute_data = { .len = 2, .data = cmd};
        g_dbus_proxy_method_call(thread->proxies.cycling_power.cycling_power_measurement_broadcast, "WriteValue", write_setup, write_reply, &write_attribute_data, NULL);
    } else if (!strncmp(command, "Broadcast measurement off", 25)) {
        uint8_t cmd[2] = {0x00, 0x00};
        struct write_attribute_data write_attribute_data = { .len = 2, .data = cmd};
        g_dbus_proxy_method_call(thread->proxies.cycling_power.cycling_power_measurement_broadcast, "WriteValue", write_setup, write_reply, &write_attribute_data, NULL);
    } else if (!strncmp(command, "Get sensor location", 19)) {
        g_dbus_proxy_method_call(thread->proxies.cycling_power.sensor_location, "ReadValue", read_setup, read_sensor_location, thread->m_frame, NULL);
    } else if (!strncmp(command, "Set cumulative value", 20)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x01, (uint32_t) atoi(&command[21]));
    } else if (!strncmp(command, "Set sensor location", 19)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x02, (uint8_t) atoi(&command[20]));
    } else if (!strncmp(command, "Get supported sensor locations", 30)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x03);
    } else if (!strncmp(command, "Set crank length", 16)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x04, (uint16_t) lround(atof(&command[17]) * 2.0));
    } else if (!strncmp(command, "Get crank length", 16)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x05);
    } else if (!strncmp(command, "Set chain length", 16)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x06, (uint16_t) atoi(&command[17]));
    } else if (!strncmp(command, "Get chain length", 16)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x07);
    } else if (!strncmp(command, "Set chain weight", 16)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x08, (uint16_t) atoi(&command[17]));
    } else if (!strncmp(command, "Get chain weight", 16)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x09);
    } else if (!strncmp(command, "Set span", 8)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x0a, (uint16_t) atoi(&command[9]));
    } else if (!strncmp(command, "Get span", 8)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x0b);
    } else if (!strncmp(command, "Start offset compensation", 25)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x0c);
    } else if (!strncmp(command, "Mask measurement", 16)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x0d, (uint16_t) atoi(&command[17]));
    } else if (!strncmp(command, "Get sampling rate", 17)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x0e);
    } else if (!strncmp(command, "Get factory calibration date", 28)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x0f);
    } else if (!strncmp(command, "Start enhanced offset compensation", 34)) {
        thread->write_proxy(thread->proxies.cycling_power.cycling_power_control_point, 0x10);
    } else if (!strncmp(command, "Notify vector on", 16)) {
        g_dbus_proxy_method_call(thread->proxies.cycling_power.cycling_power_vector, "StartNotify", NULL, notify_reply, NULL, NULL);
    } else if (!strncmp(command, "Notify vector off", 16)) {
        g_dbus_proxy_method_call(thread->proxies.cycling_power.cycling_power_vector, "StopNotify", NULL, NULL, NULL, NULL);
    } else if (!strncmp(command, "Set serial number ", 18)) {
        thread->write_proxy(thread->proxies.custom.control_point, 0x01, (const char *) &command[18]);
    } else if (!strncmp(command, "Set factory calibration date ", 29)) {
        uint16_t year = 0;
        uint8_t month = 0;
        uint8_t day = 0;
        uint8_t hour = 0;
        uint8_t minute = 0;
        uint8_t second = 0;
        sscanf(&command[29], "%hu-%hhu-%hhuT%hhu:%hhu:%hhu", &year, &month, &day, &hour, &minute, &second);
        thread->write_proxy(thread->proxies.custom.control_point, 0x02, year, month, day, hour, minute, second);
    } else if (!strncmp(command, "Set strain parameters ", 22)) {
        float k1 = 0;
        float k2 = 0;
        float k3 = 0;
        float k4 = 0;
        float k5 = 0;
        float k6 = 0;
        sscanf(&command[22], "%f %f %f %f %f %f", &k1, &k2, &k3, &k4, &k5, &k6);
        thread->write_proxy(thread->proxies.custom.control_point, 0x03, k1, k2, k3, k4, k5, k6);
    } else if (!strncmp(command, "Get strain parameters", 21)) {
        thread->write_proxy(thread->proxies.custom.control_point, 0x04);
    } else if (!strncmp(command, "Set accelerometer 1 transform ", 30)) {
        int16_t a[12] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0};
        sscanf(&command[30], "%hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8], &a[9], &a[10], &a[11]);
        thread->write_proxy(thread->proxies.custom.control_point, 0x05, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11]);
    } else if (!strncmp(command, "Get accelerometer 1 transform", 29)) {
        thread->write_proxy(thread->proxies.custom.control_point, 0x06);
    } else if (!strncmp(command, "Set accelerometer 2 transform ", 30)) {
        int16_t a[12] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0};
        sscanf(&command[30], "%hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8], &a[9], &a[10], &a[11]);
        thread->write_proxy(thread->proxies.custom.control_point, 0x07, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11]);
    } else if (!strncmp(command, "Get accelerometer 2 transform", 29)) {
        thread->write_proxy(thread->proxies.custom.control_point, 0x08);
    } else if (!strncmp(command, "Set KF parameters ", 18)) {
        float s2alpha = 0;
        float s2accel = 0;
        float driveRatio = 0;
        float r1 = 0;
        float_t r2 = 0;
        sscanf(&command[18], "%f %f %f %f %f", &s2alpha, &s2accel, &driveRatio, &r1, &r2);
        thread->write_proxy(thread->proxies.custom.control_point, 0x09, s2alpha, s2accel, driveRatio, r1, r2);
    } else if (!strncmp(command, "Get KF parameters", 17)) {
        thread->write_proxy(thread->proxies.custom.control_point, 0x0A);
    } else if (!strncmp(command, "Set partner address", 19)) {
        uint8_t ble_addr[6] = {0, 0, 0, 0, 0, 0};
        sscanf(&command[20], "%hhx %hhx %hhx %hhx %hhx %hhx", &ble_addr[0], &ble_addr[1], &ble_addr[2], &ble_addr[3], &ble_addr[4], &ble_addr[5]);
        thread->write_proxy(thread->proxies.custom.control_point, 0x0B, ble_addr[0], ble_addr[1], ble_addr[2], ble_addr[3], ble_addr[4], ble_addr[5]);
    } else if (!strncmp(command, "Get partner address", 19)) {
        thread->write_proxy(thread->proxies.custom.control_point, 0x0C);
    } else if (!strncmp(command, "Delete partner address", 22)) {
        thread->write_proxy(thread->proxies.custom.control_point, 0x0D);
    } else if (!strncmp(command, "Set cycling power vector parameters ", 36)) {
        uint8_t size = 0;
        uint8_t downsample = 0;
        sscanf(&command[36], "%hhu %hhu", &size, &downsample);
        thread->write_proxy(thread->proxies.custom.control_point, 0x0E, size, downsample);
    } else if (!strncmp(command, "Get cycling power vector parameters", 35)) {
        thread->write_proxy(thread->proxies.custom.control_point, 0x0F);




    } else if (!strncmp(command, "Notify raw on", 13)) {
        g_dbus_proxy_method_call(thread->proxies.custom.raw_data, "StartNotify", NULL, notify_reply, NULL, NULL);
    } else if (!strncmp(command, "Notify raw off", 14)) {
        g_dbus_proxy_method_call(thread->proxies.custom.raw_data, "StopNotify", NULL, NULL, NULL, NULL);
    }
    return TRUE;
}

//--------------------------------------------------------------------------------------------------
// Constructor of the thread
//--------------------------------------------------------------------------------------------------
IC2Thread::IC2Thread(IC2Frame *frame)
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    m_frame = frame;
//    proxies = (struct proxies_s *) g_malloc0(sizeof(struct proxies_s));
    memset(&proxies, 0, sizeof(struct proxies_s));
    nconnections = 0;
    quit = false;
}

//--------------------------------------------------------------------------------------------------
// Destructor of the thread
//--------------------------------------------------------------------------------------------------
IC2Thread::~IC2Thread()
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
}

//--------------------------------------------------------------------------------------------------
// Entry point of the thread
//--------------------------------------------------------------------------------------------------
wxThread::ExitCode IC2Thread::Entry()
{
    printf("\n%d %s %s\n", __LINE__, __FUNCTION__, __FILE__);
    // Create a socket for inbound communications with the user interface
    int sockfd, newsockfd, enable = 1;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        puts("ERROR opening socket");
        return 0;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        puts("setsockopt(SO_REUSEADDR) failed");
        return 0;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(9876);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "%s\n", explain_bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)));
        return 0;
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        puts("ERROR on accept");
        return 0;
    }
    puts("Thread socket connected to frame");


    // Configure the DBus connection for Bluetooth
    DBusConnection *dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);
    GDBusClient *client = g_dbus_client_new(dbus_conn, "org.bluez", "/org/bluez");

    g_dbus_client_set_connect_watch(client, connect_handler, this);
    g_dbus_client_set_disconnect_watch(client, disconnect_handler, this);
    g_dbus_client_set_signal_watch(client, message_handler, this);
    g_dbus_client_set_proxy_handlers(client, proxy_added, proxy_removed, property_changed, this);
    g_dbus_client_set_ready_watch(client, client_ready, this);


    main_loop = g_main_loop_new(NULL, FALSE);
    g_io_add_watch(g_io_channel_unix_new(newsockfd), G_IO_IN, command_dispatcher, this);
    g_main_loop_run(main_loop);



    g_dbus_client_unref(client);
    dbus_connection_unref(dbus_conn);
    g_main_loop_unref(main_loop);


    close(newsockfd);
    close(sockfd);


    puts("Finished");
    m_frame->Close(TRUE);
    return 0;
}






