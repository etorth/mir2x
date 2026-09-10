# cross-module integration tests (e.g. start server + client together and check behavior)
# go here. register one with mir2x_add_integration_test(), e.g.:
#
#   mir2x_add_integration_test(
#       NAME server_client_smoke
#       COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_run.py
#       DEPENDS server client
#       GOLD ${CMAKE_CURRENT_SOURCE_DIR}/test_run.log.gold)
#
# no cross-module integration tests exist yet, so nothing is registered here for now.
