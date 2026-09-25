# No esp_* requires on purpose: whatever the public headers need must come from
# M5UnitUnified's own REQUIRES. A dependency wrongly demoted to PRIV_REQUIRES
# upstream therefore fails here.
idf_component_register(SRCS "main.cpp" INCLUDE_DIRS ".")
