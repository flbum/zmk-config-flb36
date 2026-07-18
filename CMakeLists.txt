if(CONFIG_FLB36_STATUS_SCREEN)
  target_sources(app PRIVATE src/flb36_status_screen.c)
endif()

if(CONFIG_FLB36_I2C_TEST)
  target_sources(app PRIVATE src/flb36_i2c_test.c)
endif()
