from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from os import rename
from os.path import basename, isdir, join
from pathlib import Path
from shutil import move, copytree, copy2
from glob import glob
from distutils.version import LooseVersion

class MocConan(ConanFile):
    _default_version="local"
    _noCmakeConfigFiles = True
    _requirements = [
        "flex/2.6.4",
        "bison/3.7.1"
    ]
    name = "moc"
    homepage = "https://www.github.com/zuut/moc"
    description = "Moc, the marked-up object compiler"
    topics = ("conan", "moc", "idl", "code generation")
    url = "https://github.com/conan-io/conan-center-index"
    # see https://spdx.dev/licenses/
    license = "Apache-2.0"    
    generators = "cmake"
    settings = "os", "arch", "compiler", "build_type"
    options = { "shared": [ True, False ], "fPIC": [True, False] }
    default_options = { "shared": False, "fPIC": True }
    _cmake = None

    # picks a reasonable version if not specified
    @property
    def _version(self):
        if hasattr(self, "version") and self.version != None:
            return self.version
        if hasattr(self, "_default_version") and self._default_version != None:
            self.version = self._default_version
            return self.version
        # default to the latest version
        vmax="0"
        lvmax = LooseVersion(vmax)
        for v in self.conan_data["sources"]:
            lv = LooseVersion(v)
            try:
                if lv > lvmax:
                    vmax = v
                    lvmax = LooseVersion(vmax)
            except:
                self.output.error("unable to compare %s to %s" %(lv,lvmax))
        self.version = vmax
        return self.version

    @property
    def _source_fullpath(self):
        src_dir= join(self.source_folder,
                      self._source_subfolder).replace("\\", "/")
        return src_dir

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        self.output.info("do configure for %s" % self._version)
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("%s package is not compatible with Windows." % self.name)

    def requirements(self):
        self.output.info("calc requirements for %s" % self._version)
        for req in self._requirements:
            self.output.info( "override requirement: %s" % req )
            self.requires(req)

    # Retrieve the source code.
    def source(self):
        self.output.info("Retrieving source for moc/%s@" % self._version)
        protocol = self.conan_data["sources"][self._version].get("protocol","")
        if protocol == "":
            protocol = "local" if self._version == "local" else "url"
        self.output.info("Using protocol '%s'" % protocol)
        if protocol == "file" :
            version_info = self.conan_data["sources"][self._version]
            tarfilename=version_info["tarfilename"]
            extracted_dir=version_info["extracted_dir"]
            filename=basename(tarfilename)
            self.run( "cp %s ." % tarfilename )
            tools.unzip( filename )
            rename(extracted_dir, self._source_fullpath)
        elif protocol == "local" :
            print("source is local");
            dest_moc = join(self._source_fullpath, "moc")
            dest_uf = join(self._source_fullpath, "uf")
            print("Making directory '%s'" % dest_moc)
            Path(self._source_fullpath).mkdir(parents=True, exist_ok=True)
            copytree(join(self.recipe_folder, "moc"), dest_moc)
            copytree(join(self.recipe_folder, "uf"), dest_uf)
            copy2(join(self.recipe_folder, "CMakeLists.txt"), self._source_fullpath)
            copy2(join(self.recipe_folder, "LICENSE"), self._source_fullpath)
        else:
            #protocol == 'url'
            tools.get(**self.conan_data["sources"][self._version])
            rename(self.name + "-" + self._version, self._source_fullpath)

    # builds the project items
    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_CXX_STANDARD"] = 17
        self._cmake.definitions["Verbose"] = True
        self._cmake.configure(source_folder=self._source_subfolder)
        return self._cmake

    # the installs the project items
    def package(self):
        # export CMAKE_INSTALL_PREFIX=???
        # cmake --build . --target install
        cmake = self._configure_cmake()
        cmake.install()
        if self._noCmakeConfigFiles:
            for file in glob(self.package_folder + "/lib/**/*-config.cmake") :
                self.output.info("conan-center forbids having file %s " % file )
                move(file, file + "-sample")
        self.copy(pattern="LICENSE", dst="licenses",
                  src=self._source_subfolder,
                  keep_path=False)
        cmakeModulesDir=join(self.package_folder, "cmake-modules")
        if isdir(cmakeModulesDir):
            move(cmakeModulesDir,join(self.package_folder, "lib/cmake"))
        return

    def package_info(self):
        self.cpp_info.libs = ["uf"]
        self.env_info.MOC_TOOL = join(self.package_folder, "bin", "moc").replace("\\", "/")

        self.env_info.PATH.append(join(self.package_folder, "bin"))

        self.env_info.CMAKE_MODULE_PATH.append(join(self.package_folder, "lib", "cmake"))
        self.env_info.CMAKE_MODULE_PATH.append(join(self.package_folder, "lib", "moc"))
        self.env_info.CMAKE_MODULE_PATH.append(join(self.package_folder, "lib", "uf"))
        self.cpp_info.names["cmake_find_package"] = "Moc"
        self.cpp_info.names["cmake_find_package_multi"] = "Moc"

        self.cpp_info.builddirs.append(join("lib", "cmake"))
        self.cpp_info.builddirs.append(join("lib", "moc"))
        self.cpp_info.builddirs.append(join("lib", "uf"))
        self.cpp_info.libs = tools.collect_libs(self)
