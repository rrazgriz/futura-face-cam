#define CAM_PIXEL_FORMAT    PIXFORMAT_JPEG  //PIXFORMAT_YUV422,PIXFORMAT_GRAYSCALE,PIXFORMAT_RGB565,PIXFORMAT_JPEG
#define CAM_FRAME_SIZE      FRAMESIZE_QVGA  //QQVGA-UXGA Do not use sizes above QVGA when not JPEG
#define CAM_JPEG_QUALITY    10              //0-63 lower number means higher quality

#define CAM_DENOISE         0   // 0 = disable , 1 = enable
#define CAM_BRIGHTNESS      0   // -2 to 2
#define CAM_CONTRAST        0   // -2 to 2
#define CAM_SATURATION      0   // -2 to 2
#define CAM_EFFECT          2   // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
#define CAM_WHITEBAL        0   // 0 = disable , 1 = enable
#define CAM_AWB_GAIN        0   // 0 = disable , 1 = enable
#define CAM_WB_MODE         0   // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
#define CAM_EXPOSURE_CTRL   0   // 0 = disable , 1 = enable
#define CAM_AEC2            0   // 0 = disable , 1 = enable
#define CAM_AE_LEVEL        0   // -2 to 2
#define CAM_AEC_VALUE       100 // 0 to 1200
#define CAM_GAIN_CTRL       0   // 0 = disable , 1 = enable
#define CAM_AGC_GAIN        0   // 0 to 30
#define CAM_GAINCEILING     0   // 0 to 6
#define CAM_BPC             0   // 0 = disable , 1 = enable
#define CAM_WPC             1   // 0 = disable , 1 = enable
#define CAM_RAW_GMA         1   // 0 = disable , 1 = enable
#define CAM_LENC            1   // 0 = disable , 1 = enable
#define CAM_HMIRROR         0   // 0 = disable , 1 = enable
#define CAM_VFLIP           0   // 0 = disable , 1 = enable
#define CAM_DCW             1   // 0 = disable , 1 = enable
#define CAM_COLORBAR        0   // 0 = disable , 1 = enable