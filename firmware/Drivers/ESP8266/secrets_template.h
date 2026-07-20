/**
 * @file    secrets_template.h
 * @brief   凭据模板 — 复制为 secrets.h 并填入你的真实 WiFi / API 信息
 * @note    secrets.h 已被 .gitignore 忽略，不会上传 GitHub
 *
 * 使用方法:
 *   1. 复制本文件 → 同目录下 secrets.h
 *   2. 修改下面的占位符为你的真实值
 *   3. 编译
 */

#ifndef SECRETS_TEMPLATE_H
#define SECRETS_TEMPLATE_H

/* WiFi 配置（改成你家的 WiFi 名和密码） */
#define WIFI_SSID           "YOUR_WIFI_NAME"
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"

/* ThingSpeak API Key（从 thingspeak.com → Channels → API Keys 获取） */
#define THINGSPEAK_API_KEY  "YOUR_API_KEY"

#endif /* SECRETS_TEMPLATE_H */
