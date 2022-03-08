outputdir = "%{cfg.buildcfg}/%{cfg.system}-%{cfg.architecture}"

includeDir = {}
includeDir["glfw"] = "include/glfw"
includeDir["glew"] = "include/glew"

libDir = {}
libDir["glfw"] = "lib/glfw"
libDir["glew"] = "lib/glew"

projectName = "Project"

workspace "ProjectWorkspace" -- ChangeProject
	location "workspace"
	architecture "x86_64"

	startproject "Main" -- Change this to what's ever the starting project is 

	configurations{
		"Debug",
		"Dist",
		"Release"
	}

	pchheader "pch.hpp"
	pchsource "src/pch.cpp"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir)

	filter "system:windows"
		defines "_WINDOWS"
	filter "system:linux"
		defines "_LINUX"
		libdirs{
			"/usr/lib/x86_64-linux-gnu"
		}
	filter "system:macosx"
		defines "_OSX"
		
    filter "configurations:Debug"
        defines "_DEBUG"
        symbols "On"
        runtime "Debug"
    filter "configurations:Dist"
        defines "_DIST"
        optimize "On"
        runtime "Release"
    filter "configurations:Release"
        defines "_RELEASE"
        optimize "On"
        runtime "Release"

project "Main" -- Default Project
	language "C++"
	cppdialect "C++17"
	kind "ConsoleApp" -- ConsoleApp / SharedLib
	-- defines "_DLL" -- If SharedLib

	headerdir "include"

	files{
		"src/**.cpp",
		"src/**.c"
	}

	includedirs{
		"include",
        includeDir["glfw"],
        includeDir["glew"]
	}

    libdirs{
        libDir["glfw"],
        libDir["glew"]
    }

	links{
        "opengl32",
        "glfw3",
        "glew32",
        "glew32s"
	}

--[[
project "" -- Default Project
	language "C++"
	cppdialect "C++17"
	kind "" -- ConsoleApp / SharedLib
	-- defines "_DLL" -- If SharedLib
	
	headerdir "include"

	files{
		"src/**.cpp",
		"src/**.c"
	}

	includedirs{
		"include"
	}

	links{

	}
]]--
