#pragma once
#include <initguid.h>
#include <Mmdeviceapi.h>

static int getSampleRate() {
	HRESULT hr;
	IMMDevice *pDevice = NULL;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IPropertyStore *store = nullptr;
	PWAVEFORMATEX deviceFormatProperties;
	PROPVARIANT prop;

	(void)CoInitialize(NULL);

	// get the device enumerator
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator) ,NULL ,CLSCTX_ALL ,__uuidof(IMMDeviceEnumerator) ,(LPVOID*)&pEnumerator);

	// get default audio endpoint
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender ,eMultimedia ,&pDevice);

	hr = pDevice->OpenPropertyStore(STGM_READ ,&store);
	if (FAILED(hr)) return -1;

	hr = store->GetValue(PKEY_AudioEngine_DeviceFormat ,&prop);
	if (FAILED(hr)) return -2;

	deviceFormatProperties = (PWAVEFORMATEX)prop.blob.pBlobData;
	return deviceFormatProperties->nSamplesPerSec;
}
