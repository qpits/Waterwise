// ADC continuous mode
#include "esp_check.h"
#include "esp_log.h"
#include "esp_adc/adc_continuous.h"
#include "adc.h"

#define ADC_UNIT                    ADC_UNIT_1
#define _ADC_UNIT_STR(unit)         #unit
#define ADC_UNIT_STR(unit)          _ADC_UNIT_STR(unit)
#define ADC_CONV_MODE               ADC_CONV_SINGLE_UNIT_1
#define ADC_ATTEN                   ADC_ATTEN_DB_0
#define ADC_BIT_WIDTH               SOC_ADC_DIGI_MAX_BITWIDTH
#define SAMP_FREQ_HZ 20*1000

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif

#define ADC_READ_LEN_BYTES      256

#define SAMPLE_NUM(l)              (uint32_t)((SAMP_FREQ_HZ)*(l)/1000)

#if CONFIG_IDF_TARGET_ESP32
static adc_channel_t channel = ADC_CHANNEL_6;
#else
static adc_channel_t channel[2] = {ADC_CHANNEL_2, ADC_CHANNEL_3};
#endif

static adc_continuous_handle_t adc_cont_h;
static const char *TAG = "ADC";

static void continuous_adc_init()
{
    adc_continuous_handle_t handle = NULL;
    // initial configuration for adc continuous
    // conv_frame_size -> number of conversion results (in bytes)
    // max_store_buf_size -> max number of conversion results (in bytes)
    adc_continuous_handle_cfg_t adc_config = {
        .conv_frame_size = ADC_READ_LEN_BYTES,
        .max_store_buf_size = ADC_READ_LEN_BYTES*4
    };
    // create new handle
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    // adc IO configuration
    adc_continuous_config_t digi_conf = {
        .sample_freq_hz = SAMP_FREQ_HZ,
        .conv_mode = ADC_CONV_MODE,
        .format = ADC_OUTPUT_TYPE,
    };
    // configure every channel used
    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    digi_conf.pattern_num = 1;
    adc_pattern[0].atten = ADC_ATTEN;
    adc_pattern[0].channel = channel & 0x7;
    adc_pattern[0].unit = ADC_UNIT;
    adc_pattern[0].bit_width = ADC_BIT_WIDTH;

    ESP_LOGI(TAG, "adc atten is :%"PRIx8, adc_pattern[0].atten);
    ESP_LOGI(TAG, "adc channel is :%"PRIx8, adc_pattern[0].channel);
    ESP_LOGI(TAG, "adc unit is :%"PRIx8, adc_pattern[0].unit);
    
    digi_conf.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &digi_conf));
    adc_cont_h = handle;
}

esp_err_t adc_reading(uint32_t length_ms, adc_measure_result *result) {
    esp_err_t ret = ESP_OK;
    ESP_RETURN_ON_FALSE(length_ms > 0, ESP_ERR_INVALID_ARG, TAG, "Specified time lenght for the measure is 0.");

    continuous_adc_init();

    uint32_t samples = SAMPLE_NUM(length_ms);
    uint8_t buff[ADC_READ_LEN_BYTES] = {0};
    uint16_t *data_buff = malloc(samples * sizeof(uint16_t));
    ESP_RETURN_ON_FALSE(data_buff != NULL, ESP_ERR_NO_MEM, TAG, "Not enough memory to allocate buffer for measure.");

    uint32_t out_length;
    size_t count = 0;
    esp_err_t err = ESP_FAIL;
    ESP_LOGI(TAG, "Starting ADC measurement. Lasting approx %"PRIu32" milliseconds...", length_ms);
    ESP_ERROR_CHECK(adc_continuous_start(adc_cont_h));

    while (count < samples) {
        err = adc_continuous_read(adc_cont_h, buff, ADC_READ_LEN_BYTES, &out_length, 100);
        if (err == ESP_OK) {
            for (uint32_t i = 0; i < out_length && count < samples; i+=SOC_ADC_DIGI_RESULT_BYTES) {
                // each adc_digi_output_data_t is 2 bytes.
                // if we poke inside, there is a union between:
                //      type1 result (12 bits data, 4 bits channel);
                //      type2 results (11 bits data, 4 bits channel, 1 bit unit info);
                //      raw data (16 bit integer)
                // to read every conversion result we need to index the buffer with a stride of two bytes
                adc_digi_output_data_t *d = (adc_digi_output_data_t *)&buff[i];
                uint32_t chan_num = ADC_GET_CHANNEL(d);
                uint32_t data = ADC_GET_DATA(d);
                /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                if (chan_num < SOC_ADC_CHANNEL_NUM(ADC_UNIT)) {
                    data_buff[count++] = data;
                } else {
                    ESP_LOGW(TAG, "Invalid data [%s_%"PRIu32"_%"PRIx32"]", ADC_UNIT_STR(ADC_UNIT), chan_num, data);
                    ret = ESP_ERR_NOT_FINISHED;
                    goto clean;
                }
            }
        }
        else if (err == ESP_ERR_TIMEOUT)
            continue;
        else
            break;
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "Read %u samples from ADC at %d Hz (measure lasting %"PRIu32" ms).", count, SAMP_FREQ_HZ, length_ms);

    ESP_ERROR_CHECK(adc_continuous_stop(adc_cont_h));
    ESP_ERROR_CHECK(adc_continuous_deinit(adc_cont_h));
    result->samples = count;
    result->measure_buff = data_buff;
    return ret;

    clean:
    ESP_ERROR_CHECK(adc_continuous_stop(adc_cont_h));
    ESP_ERROR_CHECK(adc_continuous_deinit(adc_cont_h));
    result->samples = 0;
    result->measure_buff = NULL;
    free(data_buff);
    return ret;
}
