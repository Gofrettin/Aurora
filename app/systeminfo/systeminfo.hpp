#pragma once
/*
  SystemInfo class is a C++ wrapper for the System Management BIOS data available on PCs
*/

#ifndef SYSTEMINFO_HPP_
#define SYSTEMINFO_HPP_

#include <string>

class Systeminfo {
public:
    // System information data retrieved on construction and string members populated
    Systeminfo();
    // get product family
    const std::string get_family() const;
    // get manufacturer - generally motherboard or system assembler name
    const std::string get_manufacturer() const;
    // get product name
    const std::string get_productname() const;
    // get BIOS serial number
    const std::string get_serialnumber() const;
    // get SKU / system configuration
    const std::string get_sku() const;
    // get a universally unique identifier for system
    const std::string get_uuid() const;
    // get version of system information
    const std::string get_version() const;

    Systeminfo(Systeminfo const&) = delete;
    Systeminfo& operator=(Systeminfo const&) = delete;

private:
    std::string family_;
    std::string manufacturer_;
    std::string productname_;
    std::string serialnumber_;
    std::string sku_;
    std::string uuid_;
    std::string version_;
};

#endif // SYSTEMINFO_HPP_