Import("env")

# Include toolchain paths in compile_commands.json
#env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

# Write compile_commands.json to the project root (easier for clangd to find)
env.Replace(COMPILATIONDB_PATH="compile_commands.json")
