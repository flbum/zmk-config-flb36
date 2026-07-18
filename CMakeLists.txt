if(CONFIG_FLB36_STATUS_SCREEN)
  target_sources(app PRIVATE src/flb36_status_screen.c)
endif()

if(CONFIG_FLB36_CONNECTION_LED)
  target_sources(app PRIVATE src/flb36_connection_led.c)
endif()
