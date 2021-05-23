from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
import contextlib
import os


# To Test this recipe:
#
# cd $recipe_folder
# rm -rf cmake-build-debug moc_source *.tar.gz  package-folder
# conan install  . --install-folder=cmake-build-debug # [--profile XXXX]
# conan source .
# conan build .  --install-folder=cmake-build-debug
# conan package . --build-folder=cmake-build-debug --package-folder=package-folder
#
# If ok:
#
# conan create moc/{version}
#
# see: https://docs.conan.io/en/latest/reference/conanfile
class MocConan(ConanFile):
    name = "moc"
    version = "0.9.1"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://www.github.com/zuut/moc"
    description = "Moc, the marked-up object compiler"
    topics = ("conan", "moc", "idl", "code generation")
    # see https://spdx.dev/licenses/
    license = "Apache-2.0"    

    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "cmake"
    _cmake = None

    # Retrieve the source code.
    def source(self):
        print("Getting MOC version %s" % self.version);
        version_info=self.conan_data["sources"][self.version];
        extracted_dir = self.name + "-" + self.version
        protocol = version_info.get("protocol", "file")
        if protocol == "file" :
            tarfilename=version_info["tarfilename"]
            extracted_dir=version_info["extracted_dir"]
            filename=os.path.basename(tarfilename)
            self.run( "cp %s ." % tarfilename );
            tools.unzip( filename );
            os.rename(extracted_dir, self._source_subfolder)
        elif protocol == "none" :
            print("source is local");
        else:
            tools.get(**version_info)
            os.rename(extracted_dir, self._source_subfolder)

    def requirements(self):
        for req in self._program["requirements"]:
            print( "override requirement: %s" % req );
            self.requires(req);

    def configure(self):
        self._program = self.conan_data["_program"];
        #del self.settings.compiler.libcxx
        #del self.settings.compiler.cppstd
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("%s package is not compatible with Windows." % self.name)

    # builds the project items
    def build(self):
        # mkdir cmake-build-$rel
        # cd  cmake-build-$rel
        # cmake -DCMAKE_BUILD_TYPE=(release|debug) ..
        # cmake --build .
        cmake = self._cmake_init()
        cmake.build()

    # the installs the project items
    def package(self):
        # export CMAKE_INSTALL_PREFIX=???
        # cmake --build . --target install
        cmake = self._cmake_init()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["fl"]
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))

    def _cmake_init(self):
        self._cmake = CMake(self)
        if not self.settings.compiler.cppstd:
            self._cmake.definitions["CMAKE_CXX_STANDARD"] = 17
        self._cmake.definitions["Verbose"] = True
        self._cmake.configure()
        return self._cmake;

    @property
    def _source_subfolder(self):
        return "source_subfolder"
