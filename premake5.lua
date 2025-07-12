local JSON_DIR = "vendor/json"
local SPD_LOG = "vendor/spdlog"
local WEBSOCKETPP_DIR = "vendor/websocketpp"

solution "Finance"
    location("")
    startproject "terminal"
    configurations { "Release", "Debug" }
    platforms "x64"
    architecture "x64"
    
    filter "configurations:Release"
        defines {
            "_RELEASE"
        }
        optimize "Full"
    filter "configurations:Debug*"
        defines{
            "_DEBUG"
        }
        optimize "Debug"
        symbols "On"

project "market"
    location("market")
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    exceptionhandling "On"
    rtti "Off"
    warnings "Default"
    flags { "FatalWarnings", "MultiProcessorCompile" }	
    debugdir "bin"
    
    targetdir("bin/%{cfg.architecture}")
    objdir("tmp/%{cfg.architecture}")

    -- Precompiled header configuration
    pchheader "fin-pch.h"
    pchsource "market/src/fin-pch.cpp"

    disablewarnings { 
        "4057", -- Slightly different base types. Converting from type with volatile to without.
        "4100", -- Unused formal parameter. I think unusued parameters are good for documentation.
        "4152", -- Conversion from function pointer to void *. Should be ok.
        "4200", -- Zero-sized array. Valid C99.
        "4201", -- Nameless struct/union. Valid C11.
        "4204", -- Non-constant aggregate initializer. Valid C99.
        "4206", -- Translation unit is empty. Might be #ifdefed out.
        "4214", -- Bool bit-fields. Valid C99.
        "4221", -- Pointers to locals in initializers. Valid C99.
        "4702", -- Unreachable code. We sometimes want return after exit() because otherwise we get an error about no return value.
        "4996"  -- Deprecated functions
    }
  
    includedirs
    {
        path.join("%{prj.name}", JSON_DIR),
        path.join("%{prj.name}", path.join(SPD_LOG, "include") )
    }

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp"
    }

    filter "system:windows"
        links { "kernel32", "user32", "ws2_32", "Dbghelp" }

    prebuildcommands
    {
    }
