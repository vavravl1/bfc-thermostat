#include "sensors.h"
#include "vv_display.h"

static float temperature = NAN;
static float humidity = NAN;
bc_sgp30_t sgp30;

void sensors_init_all()
{
	static temperature_tag_t temperature_tag_0_0;
	temperature_tag_init(BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT, &temperature_tag_0_0);

	//----------------------------

	static humidity_tag_t humidity_tag_0_0;
	humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R1, BC_I2C_I2C0, &humidity_tag_0_0);

	static humidity_tag_t humidity_tag_0_2;
	humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R2, BC_I2C_I2C0, &humidity_tag_0_2);

	static humidity_tag_t humidity_tag_0_4;
	humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R3, BC_I2C_I2C0, &humidity_tag_0_4);

	static humidity_tag_t humidity_tag_1_0;
	humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R1, BC_I2C_I2C1, &humidity_tag_1_0);

	static humidity_tag_t humidity_tag_1_2;
	humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R2, BC_I2C_I2C1, &humidity_tag_1_2);

	static humidity_tag_t humidity_tag_1_4;
	humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R3, BC_I2C_I2C1, &humidity_tag_1_4);

	//----------------------------

	static lux_meter_tag_t lux_meter_0_0;
	lux_meter_tag_init(BC_I2C_I2C0, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT, &lux_meter_0_0);

	static lux_meter_tag_t lux_meter_0_1;
	lux_meter_tag_init(BC_I2C_I2C0, BC_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE, &lux_meter_0_1);

	static lux_meter_tag_t lux_meter_1_0;
	lux_meter_tag_init(BC_I2C_I2C1, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT, &lux_meter_1_0);

	static lux_meter_tag_t lux_meter_1_1;
	lux_meter_tag_init(BC_I2C_I2C1, BC_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE, &lux_meter_1_1);

	//----------------------------

	static barometer_tag_t barometer_tag_0_0;
	barometer_tag_init(BC_I2C_I2C0, &barometer_tag_0_0);

	static barometer_tag_t barometer_tag_1_0;
	barometer_tag_init(BC_I2C_I2C1, &barometer_tag_1_0);

	//----------------------------

	co2_module_init();

	pir_module_init();

    voc_tag_init();
}

void sgp30_event_handler(bc_sgp30_t *self, bc_sgp30_event_t event, void *event_param)
{
    if (event == BC_SGP30_EVENT_UPDATE)
    {
        uint16_t value;

        if (bc_sgp30_get_tvoc_ppb(self, &value))
        {
            int radio_tvoc = value;
            bc_radio_pub_int("voc-sensor/0:0/tvoc", &radio_tvoc);
        }
    }
}

void voc_tag_init() {
    bc_sgp30_init(&sgp30, BC_I2C_I2C0, 0x58);
    bc_sgp30_set_event_handler(&sgp30, sgp30_event_handler, NULL);
    bc_sgp30_set_update_interval(&sgp30, VOC_TAG_UPDATE_INTERVAL);
}

void tvoc_sgp30_compensation(void)
{
    static float c_temperature = -100;
    static float c_humidity = -1;

    if (isnan(temperature) || isnan(humidity))
    {
        return;
    }

    if ((fabsf(temperature - c_temperature) < 1.f) && fabsf(humidity - c_humidity) < 5.f )
    {
        return;
    }

    c_temperature = temperature;
    c_humidity = humidity;

    bc_sgp30_set_compensation(&sgp30, &temperature, &humidity);
}

static void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_temperature_get_temperature_celsius(self, &temperature))
    {
        if ((fabs(temperature - param->value) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick())) {
            bc_radio_pub_temperature(param->channel, &temperature);
        }
        vv_display_push_new_value(VV_DATA_TYPE_LOCAL_TEMPERATURE, temperature);
        tvoc_sgp30_compensation();
    }
}

void temperature_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, temperature_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.channel = i2c_address == BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT ? BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT: BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE;

    bc_tag_temperature_init(&tag->self, i2c_channel, i2c_address);

    bc_tag_temperature_set_update_interval(&tag->self, TEMPERATURE_TAG_UPDATE_INTERVAL);

    bc_tag_temperature_set_event_handler(&tag->self, temperature_tag_event_handler, &tag->param);
}

static void humidity_tag_event_handler(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param)
{
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_HUMIDITY_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_humidity_get_humidity_percentage(self, &humidity))
    {
        if ((fabs(humidity - param->value) >= HUMIDITY_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            param->value = humidity;
            param->next_pub = bc_scheduler_get_spin_tick() + HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL;
	        bc_radio_pub_humidity(param->channel, &humidity);
        }
        tvoc_sgp30_compensation();
    }
}

void humidity_tag_init(bc_tag_humidity_revision_t revision, bc_i2c_channel_t i2c_channel, humidity_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    if (revision == BC_TAG_HUMIDITY_REVISION_R1)
    {
        tag->param.channel = BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT;
    }
    else if (revision == BC_TAG_HUMIDITY_REVISION_R2)
    {
        tag->param.channel = BC_RADIO_PUB_CHANNEL_R2_I2C0_ADDRESS_DEFAULT;
    }
    else if (revision == BC_TAG_HUMIDITY_REVISION_R3)
    {
        tag->param.channel = BC_RADIO_PUB_CHANNEL_R3_I2C0_ADDRESS_DEFAULT;
    }
    else
    {
        return;
    }

    if (i2c_channel == BC_I2C_I2C1)
    {
        tag->param.channel |= 0x80;
    }

    bc_tag_humidity_init(&tag->self, revision, i2c_channel, BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);

    bc_tag_humidity_set_update_interval(&tag->self, HUMIDITY_TAG_UPDATE_INTERVAL);

    bc_tag_humidity_set_event_handler(&tag->self, humidity_tag_event_handler, &tag->param);
}

static void lux_meter_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_LUX_METER_EVENT_UPDATE)
    {
        return;
    }

    if (bc_tag_lux_meter_get_illuminance_lux(self, &value))
    {
        if ((fabs(value - param->value) >= LUX_METER_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
        {
            bc_radio_pub_luminosity(param->channel, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + LUX_METER_TAG_PUB_NO_CHANGE_INTEVAL;
        }
    }
}

void lux_meter_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address, lux_meter_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.channel = i2c_address == BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT ? BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT: BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE;

    bc_tag_lux_meter_init(&tag->self, i2c_channel, i2c_address);

    bc_tag_lux_meter_set_update_interval(&tag->self, LUX_METER_TAG_UPDATE_INTERVAL);

    bc_tag_lux_meter_set_event_handler(&tag->self, lux_meter_event_handler, &tag->param);
}

static void barometer_tag_event_handler(bc_tag_barometer_t *self, bc_tag_barometer_event_t event, void *event_param)
{
    float pascal;
    float meter;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_BAROMETER_EVENT_UPDATE)
    {
        return;
    }

    if (!bc_tag_barometer_get_pressure_pascal(self, &pascal))
    {
        return;
    }

    if ((fabs(pascal - param->value) >= BAROMETER_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
    {
        if (!bc_tag_barometer_get_altitude_meter(self, &meter))
        {
            return;
        }

        bc_radio_pub_barometer(param->channel, &pascal, &meter);
        param->value = pascal;
        param->next_pub = bc_scheduler_get_spin_tick() + BAROMETER_TAG_PUB_NO_CHANGE_INTEVAL;
    }
}

void barometer_tag_init(bc_i2c_channel_t i2c_channel, barometer_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));

    tag->param.channel = BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT;

    bc_tag_barometer_init(&tag->self, i2c_channel);

    bc_tag_barometer_set_update_interval(&tag->self, BAROMETER_TAG_UPDATE_INTERVAL);

    bc_tag_barometer_set_event_handler(&tag->self, barometer_tag_event_handler, &tag->param);
}

void co2_event_handler(bc_module_co2_event_t event, void *event_param)
{
    event_param_t *param = (event_param_t *) event_param;
    float value;

    if (event == BC_MODULE_CO2_EVENT_UPDATE)
    {
        if (bc_module_co2_get_concentration_ppm(&value))
        {
            if ((fabs(value - param->value) >= CO2_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
            {
  	        bc_radio_pub_co2(&value);
                param->value = value;
                param->next_pub = bc_scheduler_get_spin_tick() + CO2_PUB_NO_CHANGE_INTERVAL;
            }
        }
    }
}

void co2_module_init(void)
{
    static event_param_t event_param = { .next_pub = 0 };
    bc_module_co2_init();
    bc_module_co2_set_update_interval(CO2_UPDATE_INTERVAL);
    bc_module_co2_set_event_handler(co2_event_handler, &event_param);
}

static void pir_event_handler(bc_module_pir_t *self, bc_module_pir_event_t event, void*event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_MODULE_PIR_EVENT_MOTION)
    {
        static uint16_t event_count = 0;
        event_count++;
        bc_radio_pub_event_count(BC_RADIO_PUB_EVENT_PIR_MOTION, &event_count);
    }
}

void pir_module_init(void)
{
    static bc_module_pir_t pir;
    bc_module_pir_init(&pir);
    bc_module_pir_set_event_handler(&pir, pir_event_handler, NULL);
}

