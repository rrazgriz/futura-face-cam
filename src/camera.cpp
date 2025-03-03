#include "FuturaFaceTracker.h"
#include "configuration.h"
#include "camera_pins.h"
#include "camera_settings.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

void FuturaFaceTracker::configureCamera()
{
    camera_config_t configureCamera = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sscb_sda = CAM_PIN_SIOD,
        .pin_sscb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
        .xclk_freq_hz = 10000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = CAM_PIXEL_FORMAT, // PIXFORMAT_YUV422,PIXFORMAT_GRAYSCALE,PIXFORMAT_RGB565,PIXFORMAT_JPEG
        .frame_size = CAM_FRAME_SIZE,     // FRAMESIZE_240X240,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

        .jpeg_quality = CAM_JPEG_QUALITY, // 0-63 lower number means higher quality
        .fb_count = 3,                    // if more than one, i2s runs in continuous mode. Use only with JPEG
    };
    this->cameraConfig = configureCamera;
}

bool FuturaFaceTracker::initCamera()
{
    esp_err_t err = esp_camera_init(&this->cameraConfig);
    if (err != ESP_OK)
    {
        Serial.println("Camera Init Failed");
        return false;
    }
    sensor_t *s = esp_camera_sensor_get();

    s->set_denoise(s, CAM_DENOISE);
    s->set_framesize(s, CAM_FRAME_SIZE);
    s->set_brightness(s, CAM_BRIGHTNESS);
    s->set_contrast(s, CAM_CONTRAST);
    s->set_saturation(s, CAM_SATURATION);
    s->set_special_effect(s, CAM_EFFECT);
    s->set_whitebal(s, CAM_WHITEBAL);
    s->set_awb_gain(s, CAM_AWB_GAIN);
    s->set_wb_mode(s, CAM_WB_MODE);
    s->set_exposure_ctrl(s, CAM_EXPOSURE_CTRL);
    s->set_aec2(s, CAM_AEC2);           // 0 =
    s->set_ae_level(s, CAM_AE_LEVEL);   //
    s->set_aec_value(s, CAM_AEC_VALUE); // 0
    s->set_gain_ctrl(s, CAM_GAIN_CTRL); //
    s->set_agc_gain(s, CAM_AGC_GAIN);   //
    s->set_gainceiling(s, (gainceiling_t)CAM_GAINCEILING);
    s->set_bpc(s, CAM_BPC);         // 0 = disable , 1
    s->set_wpc(s, CAM_WPC);         // 0 = disable , 1
    s->set_raw_gma(s, CAM_RAW_GMA); // 0 = disable
    s->set_lenc(s, CAM_LENC);
    s->set_hmirror(s, CAM_HMIRROR);
    s->set_vflip(s, CAM_VFLIP);
    s->set_dcw(s, CAM_DCW);
    s->set_colorbar(s, CAM_COLORBAR);
    return true;
}

void FuturaFaceTracker::initStreamServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = STREAM_PORT;
    config.max_uri_handlers = 1;

    httpd_uri_t status = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = FuturaFaceTracker::streamHandler,
        .user_ctx = this};

    Serial.println("Sarting stream server");
    if (httpd_start(&this->streamServer, &config) == ESP_OK)
    {
        httpd_register_uri_handler(this->streamServer, &status);
    }
}

esp_err_t FuturaFaceTracker::streamHandler(httpd_req_t *req)
{
    FuturaFaceTracker *ft = (FuturaFaceTracker *)req->user_ctx;
    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
        return res;

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        }
        else
        {
            _timestamp.tv_sec = fb->timestamp.tv_sec;
            _timestamp.tv_usec = fb->timestamp.tv_usec;
            if (fb->format != PIXFORMAT_JPEG)
            {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if (!jpeg_converted)
                {
                    ESP_LOGE(TAG, "JPEG compression failed");
                    res = ESP_FAIL;
                }
            }
            else
            {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (fb)
        {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        }
        else if (_jpg_buf)
        {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK)
        {
            break;
        }

        // delay(100);
    }
    return res;
}
