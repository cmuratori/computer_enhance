COMPILE  :: true; // Enable to compile the library from source before generating bindings.
GENERATE :: true; // Enable to (re-)generate the Jai bindings for the C++ header.

LIBNAME_DEBUG   :: "sim86_shared_debug";
LIBNAME_RELEASE :: "sim86_shared_release";

#run {
    set_build_options_dc(.{do_output = false});

    success := true;
    #if COMPILE {
        #import "BuildCpp";

        SOURCE_FILES :: string.["../../sim86_lib.cpp"];
        ARGS :: string.["-std=c++11"];

        success &&= build_cpp_dynamic_lib(tprint("../%", LIBNAME_DEBUG),   ..SOURCE_FILES, extra = ARGS, debug = true);
        success &&= build_cpp_dynamic_lib(tprint("../%", LIBNAME_RELEASE), ..SOURCE_FILES, extra = ARGS, debug = false);
    }

    // Generate sim64.jai from the C++ header
    {
        using options: Generate_Bindings_Options;
        array_add(*libpaths, "..");
        array_add(*libnames, LIBNAME_DEBUG); // Using the debug version of the library to generate the bindings, but they can then be used for either library version
        array_add(*system_include_paths, GENERATOR_DEFAULT_SYSTEM_INCLUDE_PATH);
        array_add(*source_files,  "../sim86_shared.h");
        array_add(*extra_clang_arguments, "-x", "c++");

        alias_original_enum_names = false;
        generate_compile_time_struct_checks = false;
        generate_library_declarations = false;
        header = HEADER;
        footer = FOOTER;
        visitor = sim86_visitor;
        will_print_bindings = () {
            // Swap out the debug library name with the generic name of our custom FOOTER,
            // so that we don’t have to use confusing code like 'sim86_shared_debug :: #library "sim86_shared_release";'
            for context.generator.libraries {
                if it.name == LIBNAME_DEBUG it.name = "sim86";
            }
        };

        success &&= generate_bindings(options, "module.jai");
    }

    if !success {
        compiler_set_workspace_status(.FAILED);
        return;
    }
}

DECLARATIONS_TO_OMIT :: string.[
    "s8",
];

sim86_visitor :: (decl: *Declaration, parent_decl: *Declaration) -> Declaration_Visit_Result {
    // Omit stuff we don’t need
    if !parent_decl && array_find(DECLARATIONS_TO_OMIT, decl.name) {
        decl.decl_flags |= .OMIT_FROM_OUTPUT;
        return .STOP;
    }

    // Convert enums named "…_flag" to enum_flags
    if decl.kind == .ENUM && ends_with(decl.name, "_flag") {
        en := cast(*Enum) decl;
        en.flags |= .IS_ENUM_FLAGS;
        en.flags |= .VALUES_IN_HEX;
    }

    // Let’s be fancy and convert the enum members to their corresponding enum types:
    if decl.name == "Flags" && parent_decl.name == "instruction" {
        change_type_to_enum(decl, "instruction_flag");
    }

    return .RECURSE;
}

#import "Basic";
#import "Bindings_Generator";
#import "Compiler";
#import "String";

HEADER :: #string END
#module_parameters(DEBUG := true);
END

FOOTER :: #string END
#if DEBUG == true {
    sim86 :: #library "../sim86_shared_debug";
} else #if DEBUG == false {
    sim86 :: #library "../sim86_shared_release";
}
END
