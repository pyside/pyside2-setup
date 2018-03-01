major_version = "5"
minor_version = "6"
patch_version = "0"
pre_release_version_type = "a" # e.g. "a", "b", "rc".
pre_release_version = "1" # e.g "1", "2", (which means "beta1", "beta2", if type is "b")

if __name__ == '__main__':
    # Used by CMake.
    print('{0};{1};{2};{3};{4}'.format(major_version, minor_version, patch_version,
                                       pre_release_version_type, pre_release_version))
