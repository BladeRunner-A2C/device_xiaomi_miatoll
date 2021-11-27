/*
 * Copyright (C) 2020 The LineageOS Project
 * Copyright (C) 2021 Paranoid Android
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android-base/properties.h>
#include <sys/sysinfo.h>

#include <cstdlib>
#include <cstring>
#include <vector>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "property_service.h"
#include "vendor_init.h"

using android::base::GetProperty;

std::vector<std::string> ro_props_default_source_order = {
    "", "odm.", "product.", "system.", "vendor.", "system_ext.",
};

void property_override(char const prop[], char const value[], bool add = true) {
    auto pi = (prop_info *)__system_property_find(prop);

    if (pi != nullptr) {
        __system_property_update(pi, value, strlen(value));
    } else if (add) {
        __system_property_add(prop, strlen(prop), value, strlen(value));
    }
}

void set_ro_build_prop(const std::string &source, const std::string &prop,
                       const std::string &value, bool product = false) {
    std::string prop_name;

    if (product) {
        prop_name = "ro.product." + source + prop;
    } else {
        prop_name = "ro." + source + "build." + prop;
    }

    property_override(prop_name.c_str(), value.c_str(), false);
}

void set_device_props(const std::string brand, const std::string device, const std::string model) {
    for (const auto &source : ro_props_default_source_order) {
        set_ro_build_prop(source, "brand", brand, true);
        set_ro_build_prop(source, "device", device, true);
        set_ro_build_prop(source, "model", model, true);
    }
}

void load_device_properties() {
    property_override("ro.build.fingerprint",
                      "google/raven/raven:12/SD1A.210817.037/7862242:user/release-keys");
    property_override("ro.build.description",
                      "raven-user 12 SD1A.210817.037 7862242 release-keys");
    std::string hwname = GetProperty("ro.boot.hwname", "");
    std::string region = GetProperty("ro.boot.hwc", "");
    std::string hwversion = GetProperty("ro.boot.hwversion", "");

    if (hwname == "curtana") {
        if (region == "Global_TWO") {
            set_device_props("Redmi", "curtana", "Redmi Note 9S");
        } else if (region == "Global_PA") {
            set_device_props("Redmi", "curtana", "Redmi Note 9S");
        } else if (region == "India") {
            set_device_props("Redmi", "curtana", "Redmi Note 9 Pro");
        } else if (region == "Japan") {
            set_device_props("Redmi", "curtana", "Redmi Note 9S");
        }
    } else if (hwname == "excalibur") {
        set_device_props("Redmi", "excalibur", "Redmi Note 9 Pro Max");
    } else if (hwname == "gram") {
        set_device_props("POCO", "gram", "POCO M2 Pro");
    } else if (hwname == "joyeuse") {
        set_device_props("Redmi", "joyeuse", "Redmi Note 9 Pro");
    }
    property_override("vendor.boot.hwversion", hwversion.c_str());
    property_override("ro.boot.product.hardware.sku", hwname.c_str());
}

void load_dalvik_properties() {
    char const *heapstartsize;
    char const *heapgrowthlimit;
    char const *heapsize;
    char const *heapminfree;
    char const *heapmaxfree;
    char const *heaptargetutilization;
    struct sysinfo sys;

    sysinfo(&sys);

    if (sys.totalram >= 5ull * 1024 * 1024 * 1024) {
        // from - phone-xhdpi-6144-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "32m";
    } else if (sys.totalram >= 3ull * 1024 * 1024 * 1024) {
        // from - phone-xhdpi-4096-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "8m";
        heapmaxfree = "16m";
    } else {
        return;
    }

    property_override("dalvik.vm.heapstartsize", heapstartsize);
    property_override("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    property_override("dalvik.vm.heapsize", heapsize);
    property_override("dalvik.vm.heaptargetutilization", heaptargetutilization);
    property_override("dalvik.vm.heapminfree", heapminfree);
    property_override("dalvik.vm.heapmaxfree", heapmaxfree);
}

void load_audio_properties() {
    property_override("persist.vendor.audio.voicecall.speaker.stereo", "false");
    property_override("vendor.audio.offload.track.enable", "false");
}

void vendor_load_properties() {
    load_dalvik_properties();
    load_device_properties();
    load_audio_properties();
}
