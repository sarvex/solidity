/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

/// Unit tests for libsolidity/interface/FileReader.h

#include <libsolidity/interface/FileReader.h>

#include <test/Common.h>
#include <test/FilesystemUtils.h>
#include <test/TemporaryDirectory.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::test;


namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(FileReaderTest)

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_absolute_path)
{
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/.") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./.") == "/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/") == "/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/.") == "/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./a/") == "/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./a/.") == "/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b") == "/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/") == "/a/b/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/./b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/../a/b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/c/..") == "/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/c/../") == "/a/b/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/c/../../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/c/../../../") == "/");
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_relative_path)
{
	TemporaryDirectory tempDir("file-reader-test-");
	boost::filesystem::create_directories(tempDir.path() / "x/y/z");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "x/y/z");

	// NOTE: If path to work dir contains symlinks, boost might resolve them, making the path
	// different from tempDirPath.
	soltestAssert(boost::filesystem::current_path().is_absolute(), "");
	boost::filesystem::path tempDirPath = boost::filesystem::current_path().parent_path().parent_path().parent_path();
	soltestAssert(tempDirPath.is_absolute(), "");

	cout << "Actual: " << FileReader::normalizeCLIPathForVFS(".") << " | Expected: " << tempDirPath / "x/y/z/" << endl;
	BOOST_TEST(FileReader::normalizeCLIPathForVFS(".") == tempDirPath / "x/y/z/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./") == tempDirPath / "x/y/z/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../") == tempDirPath / "x/y/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a") == tempDirPath / "x/y/z/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/") == tempDirPath / "x/y/z/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/.") == tempDirPath / "x/y/z/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./a") == tempDirPath / "x/y/z/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./a/") == tempDirPath / "x/y/z/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./a/.") == tempDirPath / "x/y/z/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/b") == tempDirPath / "x/y/z/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/b/") == tempDirPath / "x/y/z/a/b/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../a/b") == tempDirPath / "x/y/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a/b") == tempDirPath / "x/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./a/b") == tempDirPath / "x/y/z/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("././a/b") == tempDirPath / "x/y/z/a/b");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/./b/") == tempDirPath / "x/y/z/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/../a/b/") == tempDirPath / "x/y/z/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/b/c/..") == tempDirPath / "x/y/z/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/b/c/../") == tempDirPath / "x/y/z/a/b/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a/.././../p/../q/../a/b") == tempDirPath / "a/b");

}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_redundant_slashes)
{
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("///") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("////") == "/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("////a/b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a//b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a////b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b//") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b////") == "/a/b/");
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_unc_path)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path());

	boost::filesystem::path workDir = boost::filesystem::current_path();
	soltestAssert(workDir.is_absolute(), "");

	// UNC paths start with // or \\ followed by a name. They are used for network shares on Windows.
	// On UNIX systems they are not supported but still treated in a special way.
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//host/") == "//host/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//host/a/b") == "//host/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//host/a/b/") == "//host/a/b/");

#if defined(_WIN32)
	// On Windows an UNC path can also start with \\ instead of //
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/") == "//host/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/a/b") == "//host/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/a/b/") == "//host/a/b/");
#else
	// On UNIX systems it's just a fancy relative path instead
	cout << "Actual: " << FileReader::normalizeCLIPathForVFS("\\\\host/") << " | Expected: " << workDir / "\\\\host/" << endl;
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/") == workDir / "\\\\host/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/a/b") == workDir / "\\\\host/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/a/b/") == workDir / "\\\\host/a/b/");
#endif
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_root_name_only)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path());

	boost::filesystem::path workDir = boost::filesystem::current_path();

	// A root **path** consists of a directory name (typically / or \) and the root name (drive
	// letter (C:), UNC host name (//host), etc.). Either can be empty. Root path as a whole is an
	// absolute path but root name on its own is considered relative. For example on Windows
	// C:\ represents the root directory of drive C: but C: on its own refers to the current working
	// directory.

	// UNC paths
	cout << "Actual: " << FileReader::normalizeCLIPathForVFS("//") << " | Expected: " << "//" / workDir << endl;
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//") == "//" / workDir);
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//host") == "//host" / workDir);

	// On UNIX systems root name is empty.
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("") == workDir);

#if defined(_WIN32)
	boost::filesystem::path driveLetter = workDir.root_name();
	solAssert(!driveLetter.empty(), "");
	solAssert(driveLetter.is_relative(), "");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS(driveLetter) == workDir);
#endif
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_stripping_root_name)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path());

	boost::filesystem::path workDir = boost::filesystem::current_path();
	soltestAssert(workDir.is_absolute(), "");
	soltestAssert(!workDir.root_name().empty(), "");

	boost::filesystem::path normalizedPath = FileReader::normalizeCLIPathForVFS(workDir);
	BOOST_TEST(normalizedPath == "\\" / workDir.lexically_relative(workDir.root_name()));
	BOOST_TEST(normalizedPath.root_name().empty());
	BOOST_TEST(normalizedPath.root_directory() == "\\");
}
#endif

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_path_beyond_root)
{
	TemporaryWorkingDirectory tempWorkDir("/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../.") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../a/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../../a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../../a/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../../a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/../../b/../..") == "/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../.") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../a/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/../../b/../..") == "/");
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_case_sensitivity)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path());
	boost::filesystem::create_directories(tempDir.path() / "abc");

	bool caseSensitiveFilesystem = boost::filesystem::create_directories(tempDir.path() / "ABC");
	soltestAssert(boost::filesystem::equivalent(tempDir.path() / "abc", tempDir.path() / "ABC") != caseSensitiveFilesystem, "");

	BOOST_TEST((FileReader::normalizeCLIPathForVFS((tempDir.path() / "abc")).native() == (tempDir.path() / "abc").native()));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS((tempDir.path() / "ABC")).native() == (tempDir.path() / "ABC").native()));
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_path_separators)
{
	// Even on Windows we want / as a separator.
	BOOST_TEST((FileReader::normalizeCLIPathForVFS("/a/b/c").native() == boost::filesystem::path("/a/b/c").native()));
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_should_not_resolve_symlinks)
{
	TemporaryDirectory tempDir("file-reader-test-");
	soltestAssert(tempDir.path().is_absolute(), "");
	boost::filesystem::create_directories(tempDir.path() / "abc");

	if (!createSymlinkIfSupportedByFilesystem(tempDir.path() / "abc", tempDir.path() / "sym"))
		return;

	BOOST_TEST((FileReader::normalizeCLIPathForVFS(tempDir.path() / "sym/contract.sol") == tempDir.path() / "sym/contract.sol"));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS(tempDir.path() / "abc/contract.sol") == tempDir.path() / "abc/contract.sol"));
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_may_resolve_symlinks_in_workdir_when_path_is_relative)
{
	TemporaryDirectory tempDir("file-reader-test-");
	soltestAssert(tempDir.path().is_absolute(), "");
	boost::filesystem::create_directories(tempDir.path() / "abc");

	if (!createSymlinkIfSupportedByFilesystem(tempDir.path() / "abc", tempDir.path() / "sym"))
		return;

	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "sym");
	boost::filesystem::path actualWorkingDir = boost::filesystem::current_path();

	// current_path() seems to return the with symlinks resolved. Whether it does or not,
	// normalizeCLIPathForVFS() should use the same form of the working dir for relative paths.
	BOOST_TEST((FileReader::normalizeCLIPathForVFS("contract.sol") == actualWorkingDir / "contract.sol"));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS(tempDir.path() / "sym/contract.sol") == tempDir.path() / "sym/contract.sol"));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS(tempDir.path() / "abc/contract.sol") == tempDir.path() / "abc/contract.sol"));
}

BOOST_AUTO_TEST_CASE(isPathPrefix_file_prefix)
{
	BOOST_TEST(FileReader::isPathPrefix("/", "/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/contract.sol", "/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/contract.sol/", "/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/contract.sol/.", "/contract.sol"));

	BOOST_TEST(FileReader::isPathPrefix("/", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc/def/contract.sol", "/a/bc/def/contract.sol"));

	BOOST_TEST(FileReader::isPathPrefix("/", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc/def/contract.sol", "/a/bc/def/contract.sol"));

	BOOST_TEST(!FileReader::isPathPrefix("/contract.sol", "/token.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/contract", "/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/contract.sol", "/contract"));
	BOOST_TEST(!FileReader::isPathPrefix("/contract.so", "/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/contract.sol", "/contract.so"));

	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/contract.sol", "/a/b/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/contract.sol", "/a/b/c/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/contract.sol", "/a/b/c/d/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/d/contract.sol", "/a/b/c/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/contract.sol", "/contract.sol"));
}

BOOST_AUTO_TEST_CASE(isPathPrefix_directory_prefix)
{
	BOOST_TEST(FileReader::isPathPrefix("/", "/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/", "/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c", "/"));

	BOOST_TEST(FileReader::isPathPrefix("/", "/a/bc/"));
	BOOST_TEST(FileReader::isPathPrefix("/a", "/a/bc/"));
	BOOST_TEST(FileReader::isPathPrefix("/a/", "/a/bc/"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc", "/a/bc/"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc/", "/a/bc/"));

	BOOST_TEST(!FileReader::isPathPrefix("/a", "/b/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/", "/b/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/contract.sol", "/a/b/"));

	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/", "/a/b/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c", "/a/b/"));
}

BOOST_AUTO_TEST_CASE(isPathPrefix_unc_path)
{
	BOOST_TEST(FileReader::isPathPrefix("//host/a/b/", "//host/a/b/"));
	BOOST_TEST(FileReader::isPathPrefix("//host/a/b", "//host/a/b/"));
	BOOST_TEST(FileReader::isPathPrefix("//host/a/", "//host/a/b/"));
	BOOST_TEST(FileReader::isPathPrefix("//host/a", "//host/a/b/"));
	BOOST_TEST(FileReader::isPathPrefix("//host/", "//host/a/b/"));

	// NOTE: //host and // cannot be passed to isPathPrefix() because they are considered relative.

	BOOST_TEST(!FileReader::isPathPrefix("//host1/", "//host2/"));
	BOOST_TEST(!FileReader::isPathPrefix("//host1/a/b/", "//host2/a/b/"));

	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/", "//a/b/c/"));
	BOOST_TEST(!FileReader::isPathPrefix("//a/b/c/", "/a/b/c/"));
}

BOOST_AUTO_TEST_CASE(isPathPrefix_case_sensitivity)
{
	BOOST_TEST(!FileReader::isPathPrefix("/a.sol", "/A.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/A.sol", "/a.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/A/", "/a/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/", "/A/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/BC/def/", "/a/bc/def/contract.sol"));
}

BOOST_AUTO_TEST_CASE(stripPathPrefix_file_prefix)
{
	BOOST_TEST(FileReader::stripPathPrefix("/", "/contract.sol") == "contract.sol");
	BOOST_TEST(FileReader::stripPathPrefix("/contract.sol", "/contract.sol") == ".");
	BOOST_TEST(FileReader::stripPathPrefix("/contract.sol/", "/contract.sol") == ".");
	BOOST_TEST(FileReader::stripPathPrefix("/contract.sol/.", "/contract.sol") == ".");

	BOOST_TEST(FileReader::stripPathPrefix("/", "/a/bc/def/contract.sol") == "a/bc/def/contract.sol");
	BOOST_TEST(FileReader::stripPathPrefix("/a", "/a/bc/def/contract.sol") == "bc/def/contract.sol");
	BOOST_TEST(FileReader::stripPathPrefix("/a/", "/a/bc/def/contract.sol") == "bc/def/contract.sol");
	BOOST_TEST(FileReader::stripPathPrefix("/a/bc", "/a/bc/def/contract.sol") == "def/contract.sol");
	BOOST_TEST(FileReader::stripPathPrefix("/a/bc/def/", "/a/bc/def/contract.sol") == "contract.sol");
	BOOST_TEST(FileReader::stripPathPrefix("/a/bc/def/contract.sol", "/a/bc/def/contract.sol") == ".");
}

BOOST_AUTO_TEST_CASE(stripPathPrefix_directory_prefix)
{
	BOOST_TEST(FileReader::stripPathPrefix("/", "/") == ".");

	BOOST_TEST(FileReader::stripPathPrefix("/", "/a/bc/def/") == "a/bc/def/");
	BOOST_TEST(FileReader::stripPathPrefix("/a", "/a/bc/def/") == "bc/def/");
	BOOST_TEST(FileReader::stripPathPrefix("/a/", "/a/bc/def/") == "bc/def/");
	BOOST_TEST(FileReader::stripPathPrefix("/a/bc", "/a/bc/def/") == "def/");
	BOOST_TEST(FileReader::stripPathPrefix("/a/bc/def/", "/a/bc/def/") == ".");
}

BOOST_AUTO_TEST_CASE(isUNCPath)
{
	BOOST_TEST(FileReader::isUNCPath("//"));
	BOOST_TEST(FileReader::isUNCPath("//\\"));
	BOOST_TEST(FileReader::isUNCPath("//root"));
	BOOST_TEST(FileReader::isUNCPath("//root/"));

#if defined(_WIN32)
	BOOST_TEST(FileReader::isUNCPath("\\\\"));
	BOOST_TEST(FileReader::isUNCPath("\\\\/"));
	BOOST_TEST(FileReader::isUNCPath("\\\\root"));
	BOOST_TEST(FileReader::isUNCPath("\\\\root/"));
#else
	BOOST_TEST(!FileReader::isUNCPath("\\\\"));
	BOOST_TEST(!FileReader::isUNCPath("\\\\/"));
	BOOST_TEST(!FileReader::isUNCPath("\\\\root"));
	BOOST_TEST(!FileReader::isUNCPath("\\\\root/"));
#endif

	BOOST_TEST(!FileReader::isUNCPath("\\/"));
	BOOST_TEST(!FileReader::isUNCPath("/\\"));

	BOOST_TEST(!FileReader::isUNCPath(""));
	BOOST_TEST(!FileReader::isUNCPath("."));
	BOOST_TEST(!FileReader::isUNCPath(".."));
	BOOST_TEST(!FileReader::isUNCPath("/"));
	BOOST_TEST(!FileReader::isUNCPath("a"));
	BOOST_TEST(!FileReader::isUNCPath("a/b/c"));
	BOOST_TEST(!FileReader::isUNCPath("contract.sol"));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test