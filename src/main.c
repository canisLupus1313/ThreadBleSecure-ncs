/*
 *  Copyright (c) 2022, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>

// Zephyr OpenThread integration Library
#include <zephyr/net/openthread.h>

// OpenThread API
#include <openthread/platform/ble.h>
#include <openthread/ble_secure.h>

#include "x509_cert_key.h"


LOG_MODULE_REGISTER(cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

struct openthread_context *myOpenThreadContext;
otInstance *myOpenThreadInstance = NULL;  


// BleSecure callback functions

void HandleBleSecureClientConnect(bool aConnected, void *aContext)
{
	LOG_INF("TLS Connected: %s", aConnected ? "YES" : " NO");
}


void HandleBleSecureReceive(otMessage *aMessage, void *aContext)
{
	uint16_t nLen;
	char buf[100];
	
	LOG_INF("TLS Data Received len:%d offset:%d", (int)otMessageGetLength(aMessage), (int)otMessageGetOffset(aMessage));

	//nLen = otMessageGetLength(aMessage) - otMessageGetOffset(aMessage);
    nLen = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf + 5, sizeof(buf) - 6);
	buf[nLen + 5] = 0;

	LOG_INF("Received:%s", buf + 5);
	memcpy(buf, "RECV:", 5);
	otBleSecureSend(myOpenThreadInstance, buf, strlen(buf));
}


// platform -> integrate in OT
void otPlatBleGapOnConnected(otInstance *aInstance, uint16_t aConnectionId)
{
	LOG_INF("OT Connected");
	otBleSecureConnect(myOpenThreadInstance, HandleBleSecureClientConnect, NULL);
}

void otPlatBleGapOnDisconnected(otInstance *aInstance, uint16_t aConnectionId)
{
	LOG_INF("OT Disconnected");
	otBleSecureDisconnect(myOpenThreadInstance);
}


void main(void)
{
#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_shell_uart), zephyr_cdc_acm_uart)
	int ret;
	const struct device *dev;
	uint32_t dtr = 0U;

	ret = usb_enable(NULL);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return;
	}

	dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	if (dev == NULL) {
		LOG_ERR("Failed to find specific UART device");
		return;
	}

	LOG_INF("Waiting for host to be ready to communicate");

	/* Data Terminal Ready - check if host is ready to communicate */
	while (!dtr) {
		ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		if (ret) {
			LOG_ERR("Failed to get Data Terminal Ready line state: %d",
				ret);
			continue;
		}
		k_msleep(100);
	}

	/* Data Carrier Detect Modem - mark connection as established */
	(void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
	/* Data Set Ready - the NCP SoC is ready to communicate */
	(void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
#endif

	myOpenThreadContext = openthread_get_default_context();
	myOpenThreadInstance = myOpenThreadContext->instance;

	#if CONFIG_BT
		otBleSecureSetCertificate(myOpenThreadInstance, (const uint8_t*)(OT_CLI_COAPS_X509_CERT),
                               sizeof(OT_CLI_COAPS_X509_CERT), (const uint8_t*)(OT_CLI_COAPS_PRIV_KEY),
                               sizeof(OT_CLI_COAPS_PRIV_KEY));

    	otBleSecureSetCaCertificateChain(myOpenThreadInstance,
                                      (const uint8_t*)(OT_CLI_COAPS_TRUSTED_ROOT_CERTIFICATE),
                                      sizeof(OT_CLI_COAPS_TRUSTED_ROOT_CERTIFICATE));

		otBleSecureSetSslAuthMode(myOpenThreadInstance, true);

		//otBleSecureSetPsk(myOpenThreadInstance, "SECRET", 6, "MyDevice", 8);
		otBleSecureStart(myOpenThreadInstance, HandleBleSecureReceive, false, NULL);
			
	#endif
}
