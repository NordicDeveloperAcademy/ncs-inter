# Step 2.1 - Add external project as sysbuild image for the FLPR core or Network Core
ExternalZephyrProject_Add(
  APPLICATION custom_image_name
  SOURCE_DIR ${APP_DIR}/../custom_image
  BOARD nrf54l15dk/nrf54l15/cpuflpr
  # BOARD nrf5340dk/nrf5340/cpunet
  # BOARD nrf54lm20dk/nrf54lm20a/cpuflpr
)