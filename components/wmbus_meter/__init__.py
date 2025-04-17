import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    CONF_KEY,
    CONF_PAYLOAD,
    CONF_TRIGGER_ID,
)
from esphome import automation
from esphome.components.mqtt import (
    MQTT_PUBLISH_ACTION_SCHEMA,
    MQTTPublishAction,
    mqtt_publish_action_to_code,
)

from ..wmbus_radio import RadioComponent
from ..wmbus_common import validate_driver

CONF_METER_ID = "meter_id"
CONF_RADIO_ID = "radio_id"
CONF_ON_TELEGRAM = "on_telegram"

CODEOWNERS = ["@SzczepanLeon", "@kubasaw"]

DEPENDENCIES = ["wmbus_radio"]
AUTO_LOAD = ["sensor", "text_sensor"]

MULTI_CONF = True


wmbus_meter_ns = cg.esphome_ns.namespace("wmbus_meter")
Meter = wmbus_meter_ns.class_("Meter", cg.Component)
MeterRef = Meter.operator("ref")
TelegramTrigger = wmbus_meter_ns.class_(
    "TelegramTrigger",
    automation.Trigger.template(MeterRef),
)


def hex_key_validator(key):
    try:
        key = cv.bind_key(key)
        return key
    except cv.Invalid as e:
        raise cv.Invalid(e.msg.replace("Bind key", "Key"))


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Meter),
        cv.GenerateID(CONF_RADIO_ID): cv.use_id(RadioComponent),
        cv.Required(CONF_METER_ID): cv.All(
            cv.hex_int,
            hex,
            lambda s: (s := s.removeprefix("0x")).zfill(((len(s) + 1) // 2) * 2),
        ),
        cv.Optional(CONF_TYPE, default="auto"): validate_driver,
        cv.Optional(CONF_KEY): cv.Any(
            hex_key_validator,
            cv.All(cv.string, lambda s: s.encode().hex(), hex_key_validator),
        ),
        cv.Optional(CONF_ON_TELEGRAM): automation.validate_automation(
            {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TelegramTrigger)},
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    meter = cg.new_Pvariable(config[CONF_ID])
    cg.add(
        meter.set_meter_params(
            config[CONF_METER_ID],
            config[CONF_TYPE],
            config.get(CONF_KEY, ""),
        )
    )

    radio = await cg.get_variable(config[CONF_RADIO_ID])
    cg.add(meter.set_radio(radio))
    await cg.register_component(meter, config)

    for conf in config.get(CONF_ON_TELEGRAM, []):
        trig = cg.new_Pvariable(conf[CONF_TRIGGER_ID], meter)
        await automation.build_automation(
            trig,
            [(MeterRef, "meter")],
            conf,
        )


TELEGRAM_MQTT_PUBLISH_ACTION_SCHEMA = cv.All(
    MQTT_PUBLISH_ACTION_SCHEMA.extend(
        {
            cv.Optional(CONF_PAYLOAD): cv.invalid(
                "If you want to specify payload, use generic 'mqtt.publish' action"
            )
        }
    ),
    lambda c: {**c, CONF_PAYLOAD: cv.Lambda("return meter.as_json();")},
)


automation.register_action(
    "wmbus_meter.send_telegram_with_mqtt",
    MQTTPublishAction,
    TELEGRAM_MQTT_PUBLISH_ACTION_SCHEMA,
)(mqtt_publish_action_to_code)
