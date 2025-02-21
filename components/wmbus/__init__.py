import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.log import Fore, color
from esphome.components import time
from esphome.components import mqtt
from esphome.components import wifi
from esphome.components import ethernet
from esphome.components.network import IPAddress
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_IP_ADDRESS,
    CONF_PORT,
    CONF_FORMAT,
    CONF_TIME_ID,
    CONF_FREQUENCY,
    CONF_MQTT_ID,
    CONF_MQTT,
    CONF_BROKER,
    CONF_USERNAME,
    CONF_PASSWORD,
    CONF_RETAIN,
)

from esphome.const import SOURCE_FILE_EXTENSIONS

CONF_TRANSPORT = "transport"

CONF_LED_PIN = "led_pin"
CONF_LED_BLINK_TIME = "led_blink_time"
CONF_LOG_ALL = "log_all"
CONF_ALL_DRIVERS = "all_drivers"
CONF_SYNC_MODE = "sync_mode"
CONF_INFO_COMP_ID = "info_comp_id"
CONF_WMBUS_MQTT_ID = "wmbus_mqtt_id"
CONF_CLIENTS = 'clients'
CONF_ETH_REF = "wmbus_eth_id"
CONF_WIFI_REF = "wmbus_wifi_id"
CONF_DISPLAY_ALL = 'display_all'
CONF_DISPLAY_ACTIVE = 'display_active'
CONF_BOARD = "board"

CODEOWNERS = ["@SzczepanLeon"]

DEPENDENCIES = ["time"]
AUTO_LOAD = ["sensor", "text_sensor"]

wmbus_ns = cg.esphome_ns.namespace('wmbus')
WMBusComponent = wmbus_ns.class_('WMBusComponent', cg.Component)
InfoComponent = wmbus_ns.class_('InfoComponent', cg.Component)
Client = wmbus_ns.struct('Client')
Format = wmbus_ns.enum("Format")
Transport = wmbus_ns.enum("Transport")
MqttClient = wmbus_ns.struct('MqttClient')

FORMAT = {
    "HEX": Format.FORMAT_HEX,
    "RTLWMBUS": Format.FORMAT_RTLWMBUS,
}
validate_format = cv.enum(FORMAT, upper=True)

TRANSPORT = {
    "TCP": Transport.TRANSPORT_TCP,
    "UDP": Transport.TRANSPORT_UDP,
}
validate_transport = cv.enum(TRANSPORT, upper=True)

# T3S3  Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN);
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              3
#define RADIO_MOSI_PIN              6
#define RADIO_CS_PIN                7
#define RADIO_RST_PIN               8
#define RADIO_DIO0_PIN              9
#define RADIO_DIO1_PIN              33

# Elite Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN);
#define RADIO_SCLK_PIN              10
#define RADIO_MISO_PIN              9
#define RADIO_MOSI_PIN              11
#define RADIO_CS_PIN                40
#define RADIO_RST_PIN               46
#define RADIO_DIO0_PIN              8
#define RADIO_DIO1_PIN              16

BOARD = {
    "T3S3":       {"RADIO_TYPE": "SX1276", "RADIO_SCLK_PIN": 5,  "RADIO_MISO_PIN": 3, "RADIO_MOSI_PIN": 6,  "RADIO_CS_PIN": 7,
                                           "RADIO_RST_PIN": 8,  "RADIO_DIO0_PIN": 9, "RADIO_DIO1_PIN": 33,
                                           "LED_PIN": 38, "I2C_SDA": 18, "I2C_SCL": 17},
    "ELITE":      {"RADIO_TYPE": "SX1276", "RADIO_SCLK_PIN": 10, "RADIO_MISO_PIN": 9, "RADIO_MOSI_PIN": 11, "RADIO_CS_PIN": 40,
                                           "RADIO_RST_PIN": 46, "RADIO_DIO0_PIN": 8, "RADIO_DIO1_PIN": 16,
                                           "LED_PIN": 38, "I2C_SDA": 17, "I2C_SCL": 18},
}
validate_board = cv.enum(BOARD, upper=True)

CLIENT_SCHEMA = cv.Schema({
    cv.GenerateID():                              cv.declare_id(Client),
    cv.Required(CONF_NAME):                       cv.string_strict,
    cv.Required(CONF_IP_ADDRESS):                 cv.ipv4address,
    cv.Required(CONF_PORT):                       cv.port,
    cv.Optional(CONF_TRANSPORT, default="TCP"):   cv.templatable(validate_transport),
    cv.Optional(CONF_FORMAT, default="RTLWMBUS"): cv.templatable(validate_format),
})

WMBUS_MQTT_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_WMBUS_MQTT_ID):        cv.declare_id(MqttClient),
    cv.Required(CONF_USERNAME):               cv.string_strict,
    cv.Required(CONF_PASSWORD):               cv.string_strict,
    cv.Required(CONF_BROKER):                 cv.ipv4address,
    cv.Optional(CONF_PORT,    default=1883):  cv.port,
    cv.Optional(CONF_RETAIN,  default=False): cv.boolean,
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_INFO_COMP_ID):                  cv.declare_id(InfoComponent),
    cv.GenerateID():                                   cv.declare_id(WMBusComponent),
    cv.OnlyWith(CONF_MQTT_ID, "mqtt"):                 cv.use_id(mqtt.MQTTClientComponent),
    cv.OnlyWith(CONF_TIME_ID, "time"):                 cv.use_id(time.RealTimeClock),
    cv.OnlyWith(CONF_WIFI_REF, "wifi"):                cv.use_id(wifi.WiFiComponent),
    cv.OnlyWith(CONF_ETH_REF, "ethernet"):             cv.use_id(ethernet.EthernetComponent),
    cv.Required(CONF_BOARD):                           cv.templatable(validate_board),
    cv.Optional(CONF_LED_PIN,        default=38):      pins.gpio_output_pin_schema,
    cv.Optional(CONF_LED_BLINK_TIME, default="200ms"): cv.positive_time_period,
    cv.Optional(CONF_LOG_ALL,        default=False):   cv.boolean,
    cv.Optional(CONF_DISPLAY_ALL,    default=True):    cv.boolean,
    cv.Optional(CONF_DISPLAY_ACTIVE, default=True):    cv.boolean,
    cv.Optional(CONF_ALL_DRIVERS,    default=False):   cv.boolean,
    cv.Optional(CONF_CLIENTS):                         cv.ensure_list(CLIENT_SCHEMA),
    cv.Optional(CONF_FREQUENCY,      default=868.950): cv.float_range(min=300, max=928),
    cv.Optional(CONF_SYNC_MODE,      default=False):   cv.boolean,
    cv.Optional(CONF_MQTT):                            cv.ensure_schema(WMBUS_MQTT_SCHEMA),
})

def safe_ip(ip):
    if ip is None:
        return IPAddress(0, 0, 0, 0)
    return IPAddress(str(ip))

async def to_code(config):
    var_adv = cg.new_Pvariable(config[CONF_INFO_COMP_ID])
    await cg.register_component(var_adv, {})

    if (config.get(CONF_MQTT_ID) and config.get(CONF_MQTT)):
        print(color(Fore.RED, "Only one MQTT can be configured!"))
        exit()

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    for define, value in BOARD[config[CONF_BOARD]].items():
        if isinstance(value, str):
            cg.add_define(define, cg.RawExpression(value))
        else:
            cg.add_define(define, (value))

    cg.add(var.add_cc1101(0, 0, 0, 0, 0, 0, config[CONF_FREQUENCY], config[CONF_SYNC_MODE]))

    time = await cg.get_variable(config[CONF_TIME_ID])
    cg.add(var.set_time(time))


    if config.get(CONF_ETH_REF):
        eth = await cg.get_variable(config[CONF_ETH_REF])
        cg.add(var.set_eth(eth))

    if config.get(CONF_WIFI_REF):
        wifi = await cg.get_variable(config[CONF_WIFI_REF])
        cg.add(var.set_wifi(wifi))

    if config.get(CONF_MQTT_ID):
        mqtt = await cg.get_variable(config[CONF_MQTT_ID])
        cg.add(var.set_mqtt(mqtt))

    if config.get(CONF_MQTT):
        conf = config.get(CONF_MQTT)
        cg.add_define("USE_WMBUS_MQTT")
        cg.add_library("knolleary/PubSubClient", "2.8")
        cg.add(var.set_mqtt(conf[CONF_USERNAME],
                            conf[CONF_PASSWORD],
                            safe_ip(conf[CONF_BROKER]),
                            conf[CONF_PORT],
                            conf[CONF_RETAIN]))

    cg.add(var.set_log_all(config[CONF_LOG_ALL]))
    cg.add(var.set_display_all(config[CONF_DISPLAY_ALL]))
    cg.add(var.activate_display(config[CONF_DISPLAY_ACTIVE]))

    for conf in config.get(CONF_CLIENTS, []):
        cg.add(var.add_client(conf[CONF_NAME],
                              safe_ip(conf[CONF_IP_ADDRESS]),
                              conf[CONF_PORT],
                              conf[CONF_TRANSPORT],
                              conf[CONF_FORMAT]))

    if CONF_LED_PIN in config:
        led_pin = await cg.gpio_pin_expression(config[CONF_LED_PIN])
        cg.add(var.set_led_pin(led_pin))
        cg.add(var.set_led_blink_time(config[CONF_LED_BLINK_TIME].total_milliseconds))

    cg.add_library("SPI", None)
    cg.add_library("EEPROM", None)
    cg.add_library("ricmoo/QRCode", "0.0.1")
    cg.add_library("olikraus/U8g2", "2.35.27")
    cg.add_library("jgromes/RadioLib", "7.1.1")
    cg.add_platformio_option("lib_ldf_mode", "chain")

    cg.add_platformio_option("build_src_filter", ["+<*>", "-<.git/>", "-<.svn/>"])

    if config[CONF_ALL_DRIVERS]:
        cg.add_platformio_option("build_src_filter", ["+<**/wmbus/driver_*.cpp>"])
    else:
        cg.add_platformio_option("build_src_filter", ["-<**/wmbus/driver_*.cpp>"])

    cg.add_platformio_option("build_src_filter", ["+<**/wmbus/driver_unknown.cpp>"])

