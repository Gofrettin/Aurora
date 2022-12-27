#include "systeminfo.hpp"

#ifndef _WIN32
#error Currently SystemInfo can only be built for the Windows platform
#endif

#include <Windows.h>

#include <cstdint>
#include <cstdio>
#include <string>

namespace {
#pragma pack(push) 
#pragma pack(1)
    /*
    SMBIOS Structure header (System Management BIOS) spec:
    https ://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf
    */
    struct SMBIOSHEADER
    {
        uint8_t type;
        uint8_t length;
        uint16_t handle;
    };

    /*
      Structure needed to get the SMBIOS table using GetSystemFirmwareTable API.
      see https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
    */
    struct SMBIOSData {
        uint8_t  Used20CallingMethod;
        uint8_t  SMBIOSMajorVersion;
        uint8_t  SMBIOSMinorVersion;
        uint8_t  DmiRevision;
        uint32_t  Length;
        uint8_t  SMBIOSTableData[1];
    };

    // System Information (Type 1)
    struct SYSTEMINFORMATION {
        SMBIOSHEADER Header;
        uint8_t Manufacturer;
        uint8_t ProductName;
        uint8_t Version;
        uint8_t SerialNumber;
        uint8_t UUID[16];
        uint8_t WakeUpType;  // Identifies the event that caused the system to power up
        uint8_t SKUNumber;   // identifies a particular computer configuration for sale
        uint8_t Family;
    };
#pragma pack(pop) 

    // helper to retrieve string at string offset. Optional null string description can be set.
    const char* get_string_by_index(const char* str, int index, const char* null_string_text = "")
    {
        if (0 == index || 0 == *str) {
            return null_string_text;
        }

        while (--index) {
            str += strlen(str) + 1;
        }
        return str;
    }

    // retrieve the BIOS data block from the system
    SMBIOSData* get_bios_data() {
        SMBIOSData* bios_data = nullptr;

        // GetSystemFirmwareTable with arg RSMB retrieves raw SMBIOS firmware table
        // return value is either size of BIOS table or zero if function fails
        DWORD bios_size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);

        if (bios_size > 0) {
            bios_data = (SMBIOSData*)malloc(bios_size);

            // Retrieve the SMBIOS table
            DWORD bytes_retrieved = GetSystemFirmwareTable('RSMB', 0, bios_data, bios_size);

            if (bytes_retrieved != bios_size) {
                free(bios_data);
                bios_data = nullptr;
            }
        }

        return bios_data;
    }


    // locates system information memory block in BIOS table
    SYSTEMINFORMATION* find_system_information(SMBIOSData* bios_data) {

        uint8_t* data = bios_data->SMBIOSTableData;

        while (data < bios_data->SMBIOSTableData + bios_data->Length)
        {
            uint8_t* next;
            SMBIOSHEADER* header = (SMBIOSHEADER*)data;

            if (header->length < 4)
                break;

            //Search for System Information structure with type 0x01 (see para 7.2)
            if (header->type == 0x01 && header->length >= 0x19)
            {
                return (SYSTEMINFORMATION*)header;
            }

            //skip over formatted area
            next = data + header->length;

            //skip over unformatted area of the structure (marker is 0000h)
            while (next < bios_data->SMBIOSTableData + bios_data->Length && (next[0] != 0 || next[1] != 0)) {
                next++;
            }
            next += 2;

            data = next;
        }
        return nullptr;
    }

}

Systeminfo::Systeminfo() {
    SMBIOSData* bios_data = get_bios_data();

    if (bios_data) {
        SYSTEMINFORMATION* sysinfo = find_system_information(bios_data);
        if (sysinfo) {
            const char* str = (const char*)sysinfo + sysinfo->Header.length;

            manufacturer_ = get_string_by_index(str, sysinfo->Manufacturer);
            productname_ = get_string_by_index(str, sysinfo->ProductName);
            serialnumber_ = get_string_by_index(str, sysinfo->SerialNumber);
            version_ = get_string_by_index(str, sysinfo->Version);

            // for v2.1 and later
            if (sysinfo->Header.length > 0x08)
            {
                static const int max_uuid_size{ 50 };
                char uuid[max_uuid_size] = {};
                _snprintf_s(uuid, max_uuid_size, max_uuid_size - 1, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                    sysinfo->UUID[0], sysinfo->UUID[1], sysinfo->UUID[2], sysinfo->UUID[3],
                    sysinfo->UUID[4], sysinfo->UUID[5], sysinfo->UUID[6], sysinfo->UUID[7],
                    sysinfo->UUID[8], sysinfo->UUID[9], sysinfo->UUID[10], sysinfo->UUID[11],
                    sysinfo->UUID[12], sysinfo->UUID[13], sysinfo->UUID[14], sysinfo->UUID[15]);

                uuid_ = uuid;
            }

            if (sysinfo->Header.length > 0x19)
            {
                // supported in v 2.4 spec
                sku_ = get_string_by_index(str, sysinfo->SKUNumber);
                family_ = get_string_by_index(str, sysinfo->Family);
            }
        }
        free(bios_data);
    }
}

const std::string Systeminfo::get_family() const {
    return family_;
}

const std::string Systeminfo::get_manufacturer() const {
    return manufacturer_;
}

const std::string Systeminfo::get_productname() const {
    return productname_;
}

const std::string Systeminfo::get_serialnumber() const {
    return serialnumber_;
}

const std::string Systeminfo::get_sku() const {
    return sku_;
}

const std::string Systeminfo::get_uuid() const {
    return uuid_;
}

const std::string Systeminfo::get_version() const {
    return version_;
}