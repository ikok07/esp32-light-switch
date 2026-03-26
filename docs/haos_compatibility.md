#### To make the device compatible with [Home Assistant integration](https://github.com/ikok07/kok-products-ble-haos-integration) you should configure the following BLE parameters:

----

1. Identifier service should be set in the advertisement fields (check UUID from the [integration's repo](https://github.com/ikok07/kok-products-ble-haos-integration))
2. The encryption mode should be set to "Just Works" (integration doesn't support other methods yet)
3. Advertised manufacturer data:
   - byte 1–2 ⇒ Manufacturer ID (check from the [integration's repo](https://github.com/ikok07/kok-products-ble-haos-integration))
   - byte 3 ⇒ Device Model (must be implemented in [integration's repo](https://github.com/ikok07/kok-products-ble-haos-integration))
   - byte 4 ⇒ If the device requires pairing (0 or 1)