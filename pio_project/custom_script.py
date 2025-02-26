# Remove compile target
Import("env")

EXCLUDE_FILES = (env.GetProjectOption("custom_exclude_src_files") or []).split(' ')


def skip_from_build(node):
  src_path = node.get_abspath()
  #filename = os.path.basename(src_path)

  #print(EXCLUDE_FILES)
  #print('>>>> {}'.format(src_path))
#  for ef in EXCLUDE_FILES:
#    if 'I2C_Class.cpp' in src_path and 'I2C_Class.cpp' in ef:
#        print('>>>> {} {}'.format(src_path, ef))
        
  if any(ef in src_path for ef in EXCLUDE_FILES):
    return None
  return node

env.AddBuildMiddleware(skip_from_build, "*.cpp")
