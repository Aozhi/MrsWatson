//
// ProgramOptionTest.c - MrsWatson
// Copyright (c) 2016 Teragon Audio. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include "app/ProgramOption.h"

#include "base/File.h"
#include "unit/TestRunner.h"

#define TEST_CONFIG_FILE "mrswatsontest-config.txt"

static void _programOptionTeardown(void) {
  CharString configFilePath = newCharStringWithCString(TEST_CONFIG_FILE);
  File configFile = newFileWithPath(configFilePath);

  if (fileExists(configFile)) {
    fileRemove(configFile);
  }

  freeCharString(configFilePath);
  freeFile(configFile);
}

static ProgramOption _getTestOption(void) {
  return newProgramOptionWithName(0, "test", "test help", true,
                                  kProgramOptionTypeNumber,
                                  kProgramOptionArgumentTypeOptional);
}

static int _testNewProgramOptions(void) {
  ProgramOptions p = newProgramOptions(4);
  assertIntEquals(4, p->numOptions);
  freeProgramOptions(p);
  return 0;
}

static int _testAddNewProgramOption(void) {
  ProgramOptions p = newProgramOptions(1);
  ProgramOption o;
  assert(programOptionsAdd(p, _getTestOption()));
  assertIntEquals(1, p->numOptions);
  o = p->options[0];
  assertNotNull(o);
  assertCharStringEquals("test", o->name);
  freeProgramOptions(p);
  return 0;
}

static int _testAddNullProgramOption(void) {
  ProgramOptions p = newProgramOptions(1);
  assertFalse(programOptionsAdd(p, NULL));
  freeProgramOptions(p);
  return 0;
}

static int _testAddNewProgramOptionOutsideRange(void) {
  ProgramOptions p = newProgramOptions(1);
  ProgramOption o = _getTestOption();
  o->index++;
  assertFalse(programOptionsAdd(p, o));
  assertIntEquals(1, p->numOptions);
  freeProgramOption(o);
  freeProgramOptions(p);
  return 0;
}

static int _testParseCommandLineShortOption(void) {
  ProgramOptions p = newProgramOptions(1);
  char *argv[2];
  argv[0] = "exe";
  argv[1] = "-t";
  assert(programOptionsAdd(p, _getTestOption()));
  assertFalse(p->options[0]->enabled);
  assert(programOptionsParseArgs(p, 2, argv));
  assert(p->options[0]->enabled);
  freeProgramOptions(p);
  return 0;
}

static int _testParseCommandLineLongOption(void) {
  ProgramOptions p = newProgramOptions(1);
  char *argv[2];
  argv[0] = "exe";
  argv[1] = "--test";
  assert(programOptionsAdd(p, _getTestOption()));
  assertFalse(p->options[0]->enabled);
  assert(programOptionsParseArgs(p, 2, argv));
  assert(p->options[0]->enabled);
  freeProgramOptions(p);
  return 0;
}

static int _testParseCommandLineInvalidOption(void) {
  ProgramOptions p = newProgramOptions(1);
  char *argv[2];
  argv[0] = "exe";
  argv[1] = "invalid";
  assert(programOptionsAdd(p, _getTestOption()));
  assertFalse(p->options[0]->enabled);
  assertFalse(programOptionsParseArgs(p, 2, argv));
  assertFalse(p->options[0]->enabled);
  freeProgramOptions(p);
  return 0;
}

static int _testParseCommandLineRequiredOption(void) {
  ProgramOptions p = newProgramOptions(1);
  ProgramOption o = _getTestOption();
  char *argv[3];
  argv[0] = "exe";
  argv[1] = "--test";
  argv[2] = "1.23";
  o->argumentType = kProgramOptionArgumentTypeRequired;
  assert(programOptionsAdd(p, o));

  assertFalse(p->options[0]->enabled);
  assertDoubleEquals(0.0, programOptionsGetNumber(p, 0), 0.0f);
  assert(programOptionsParseArgs(p, 3, argv));
  assertDoubleEquals(1.23, programOptionsGetNumber(p, 0),
                     TEST_DEFAULT_TOLERANCE);
  assert(p->options[0]->enabled);

  freeProgramOptions(p);
  return 0;
}

static int _testParseCommandLineRequiredOptionMissing(void) {
  ProgramOptions p = newProgramOptions(1);
  ProgramOption o = _getTestOption();
  char *argv[2];
  argv[0] = "exe";
  argv[1] = "--test";
  o->argumentType = kProgramOptionArgumentTypeRequired;
  assert(programOptionsAdd(p, o));

  assertFalse(p->options[0]->enabled);
  assertDoubleEquals(0.0, programOptionsGetNumber(p, 0), TEST_EXACT_TOLERANCE);
  assertFalse(programOptionsParseArgs(p, 2, argv));
  assertFalse(p->options[0]->enabled);

  freeProgramOptions(p);
  return 0;
}

static ProgramOptions _getTestProgramOptionsForConfigFile(void) {
  ProgramOptions p = newProgramOptions(2);
  ProgramOption o1, o2;
  o1 = newProgramOptionWithName(0, "test", "test help", true,
                                kProgramOptionTypeString,
                                kProgramOptionArgumentTypeNone);
  programOptionsAdd(p, o1);
  o2 = newProgramOptionWithName(1, "other", "test help", true,
                                kProgramOptionTypeString,
                                kProgramOptionArgumentTypeRequired);
  programOptionsAdd(p, o2);
  return p;
}

static FILE *_openTestProgramConfigFile(void) {
  FILE *fp = fopen(TEST_CONFIG_FILE, "w");
  return fp;
}

static int _testParseConfigFile(void) {
  ProgramOptions p = _getTestProgramOptionsForConfigFile();
  CharString filename = newCharStringWithCString(TEST_CONFIG_FILE);
  FILE *fp = _openTestProgramConfigFile();
  assertNotNull(fp);
  fprintf(fp, "--test\n-o\nfoo\n");
  fclose(fp);
  assert(programOptionsParseConfigFile(p, filename));
  assert(p->options[0]->enabled);
  assert(p->options[1]->enabled);
  assertCharStringEquals("foo", programOptionsGetString(p, 1));

  unlink(TEST_CONFIG_FILE);
  freeProgramOptions(p);
  freeCharString(filename);
  return 0;
}

static int _testParseInvalidConfigFile(void) {
  ProgramOptions p = newProgramOptions(1);
  CharString filename = newCharStringWithCString("invalid");
  assertFalse(programOptionsParseConfigFile(p, filename));
  freeProgramOptions(p);
  freeCharString(filename);
  return 0;
}

static int _testParseNullConfigFile(void) {
  ProgramOptions p = newProgramOptions(1);
  assertFalse(programOptionsParseConfigFile(p, NULL));
  freeProgramOptions(p);
  return 0;
}

static int _testParseConfigFileWithInvalidOptions(void) {
  ProgramOptions p = _getTestProgramOptionsForConfigFile();
  CharString filename = newCharStringWithCString(TEST_CONFIG_FILE);
  FILE *fp = _openTestProgramConfigFile();
  assertNotNull(fp);
  fprintf(fp, "--test\n-s\n");
  fclose(fp);
  assertFalse(programOptionsParseConfigFile(p, filename));
  assert(p->options[0]->enabled);
  assertFalse(p->options[1]->enabled);

  unlink(TEST_CONFIG_FILE);
  freeProgramOptions(p);
  freeCharString(filename);
  return 0;
}

static int _testFindProgramOptionFromString(void) {
  ProgramOptions p = newProgramOptions(1);
  CharString c = newCharStringWithCString("test");
  ProgramOption o;

  assert(programOptionsAdd(p, _getTestOption()));
  assertIntEquals(p->numOptions, 1);
  o = programOptionsFind(p, c);
  assertNotNull(o);
  assertCharStringEquals("test", o->name);

  freeProgramOptions(p);
  freeCharString(c);
  return 0;
}

static int _testFindProgramOptionFromStringInvalid(void) {
  ProgramOptions p = newProgramOptions(1);
  CharString c = newCharStringWithCString("invalid");
  ProgramOption o;

  assert(programOptionsAdd(p, _getTestOption()));
  assertIntEquals(1, p->numOptions);
  o = programOptionsFind(p, c);
  assertIsNull(o);

  freeProgramOptions(p);
  freeCharString(c);
  return 0;
}

static ProgramOptions _getTestOptionMultipleTypes(void) {
  ProgramOptions p = newProgramOptions(3);
  programOptionsAdd(p, newProgramOptionWithName(
                           0, "string", "help", true, kProgramOptionTypeString,
                           kProgramOptionArgumentTypeRequired));
  programOptionsAdd(p, newProgramOptionWithName(
                           1, "numeric", "help", true, kProgramOptionTypeNumber,
                           kProgramOptionArgumentTypeRequired));
  programOptionsAdd(p, newProgramOptionWithName(
                           2, "list", "help", true, kProgramOptionTypeList,
                           kProgramOptionArgumentTypeRequired));
  return p;
}

static int _testGetString(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s = programOptionsGetString(p, 0);
  assertCharStringEquals("", s);
  freeProgramOptions(p);
  return 0;
}

static int _testGetStringForWrongType(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s = programOptionsGetString(p, 1);
  assertIsNull(s);
  freeProgramOptions(p);
  return 0;
}

static int _testGetStringForInvalidOption(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s = programOptionsGetString(p, 4);
  assertIsNull(s);
  freeProgramOptions(p);
  return 0;
}

static int _testGetNumeric(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  float f = programOptionsGetNumber(p, 1);
  assertDoubleEquals(0.0, f, TEST_DEFAULT_TOLERANCE);
  freeProgramOptions(p);
  return 0;
}

static int _testGetNumericForWrongType(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  float f = programOptionsGetNumber(p, 0);
  assertDoubleEquals(-1.0, f, TEST_DEFAULT_TOLERANCE);
  freeProgramOptions(p);
  return 0;
}

static int _testGetNumericForInvalidOption(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  float f = programOptionsGetNumber(p, 4);
  assertDoubleEquals(-1.0, f, TEST_DEFAULT_TOLERANCE);
  freeProgramOptions(p);
  return 0;
}

static int _testGetList(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  LinkedList l = programOptionsGetList(p, 2);
  assertIntEquals(0, l->_numItems);
  freeProgramOptions(p);
  return 0;
}

static int _testGetListForWrongType(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  LinkedList l = programOptionsGetList(p, 0);
  assertIsNull(l);
  freeProgramOptions(p);
  return 0;
}

static int _testGetListForInvalidOption(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  LinkedList l = programOptionsGetList(p, 4);
  assertIsNull(l);
  freeProgramOptions(p);
  return 0;
}

static int _testSetString(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s;
  programOptionsSetCString(p, 0, "test");
  s = programOptionsGetString(p, 0);
  assertNotNull(s);
  assertCharStringEquals("test", s);
  freeProgramOptions(p);
  return 0;
}

static int _testSetStringForWrongType(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  programOptionsSetCString(p, 1, "test");
  freeProgramOptions(p);
  return 0;
}

static int _testSetStringForInvalidOption(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s;
  programOptionsSetCString(p, 4, "test");
  s = programOptionsGetString(p, 0);
  assertNotNull(s);
  assertCharStringEquals("", s);
  freeProgramOptions(p);
  return 0;
}

static int _testSetStringWithNull(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s;
  programOptionsSetCString(p, 0, NULL);
  s = programOptionsGetString(p, 0);
  assertNotNull(s);
  assertCharStringEquals("", s);
  freeProgramOptions(p);
  return 0;
}

static int _testSetNumeric(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  float f;
  programOptionsSetNumber(p, 1, 1.23f);
  f = programOptionsGetNumber(p, 1);
  assertDoubleEquals(1.23, f, TEST_DEFAULT_TOLERANCE);
  freeProgramOptions(p);
  return 0;
}

static int _testSetNumericForWrongType(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  programOptionsSetNumber(p, 0, 1.23f);
  freeProgramOptions(p);
  return 0;
}

static int _testSetNumericForInvalidOption(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  programOptionsSetNumber(p, 4, 1.23f);
  freeProgramOptions(p);
  return 0;
}

static int _testSetListItem(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s;
  LinkedList l;
  CharString r;

  s = newCharStringWithCString("test");
  programOptionsSetListItem(p, 2, s);
  l = programOptionsGetList(p, 2);
  assertIntEquals(1, linkedListLength(l));
  r = l->item;
  assertCharStringEquals("test", r);
  freeCharString(s);
  freeProgramOptions(p);
  return 0;
}

static int _testSetListItemForWrongType(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s = newCharStringWithCString("test");
  programOptionsSetListItem(p, 1, s);
  freeCharString(s);
  freeProgramOptions(p);
  return 0;
}

static int _testSetListItemForInvalidOption(void) {
  ProgramOptions p = _getTestOptionMultipleTypes();
  CharString s = newCharStringWithCString("test");
  programOptionsSetListItem(p, 4, s);
  freeCharString(s);
  freeProgramOptions(p);
  return 0;
}

TestSuite addProgramOptionTests(void);
TestSuite addProgramOptionTests(void) {
  TestSuite testSuite =
      newTestSuite("ProgramOption", NULL, _programOptionTeardown);
  addTest(testSuite, "NewObject", _testNewProgramOptions);
  addTest(testSuite, "AddNewProgramOption", _testAddNewProgramOption);
  addTest(testSuite, "AddNullProgramOption", _testAddNullProgramOption);
  addTest(testSuite, "AddNewProgramOptionOutsideRange",
          _testAddNewProgramOptionOutsideRange);

  addTest(testSuite, "ParseCommandLineShortOption",
          _testParseCommandLineShortOption);
  addTest(testSuite, "ParseCommandLineLongOption",
          _testParseCommandLineLongOption);
  addTest(testSuite, "ParseCommandLineInvalidOption",
          _testParseCommandLineInvalidOption);
  addTest(testSuite, "ParseCommandLineRequiredOption",
          _testParseCommandLineRequiredOption);
  addTest(testSuite, "ParseCommandLineRequiredOptionMissing",
          _testParseCommandLineRequiredOptionMissing);

  addTest(testSuite, "ParseConfigFile", _testParseConfigFile);
  addTest(testSuite, "ParseInvalidConfigFile", _testParseInvalidConfigFile);
  addTest(testSuite, "ParseNullConfigFile", _testParseNullConfigFile);
  addTest(testSuite, "ParseConfigFileWithInvalidOptions",
          _testParseConfigFileWithInvalidOptions);

  addTest(testSuite, "FindProgramOptionFromString",
          _testFindProgramOptionFromString);
  addTest(testSuite, "FindProgramOptionFromStringInvalid",
          _testFindProgramOptionFromStringInvalid);

  addTest(testSuite, "GetString", _testGetString);
  addTest(testSuite, "GetStringForWrongType", _testGetStringForWrongType);
  addTest(testSuite, "GetStringForInvalidOption",
          _testGetStringForInvalidOption);
  addTest(testSuite, "GetNumeric", _testGetNumeric);
  addTest(testSuite, "GetNumericForWrongType", _testGetNumericForWrongType);
  addTest(testSuite, "GetNumericForInvalidOption",
          _testGetNumericForInvalidOption);
  addTest(testSuite, "GetList", _testGetList);
  addTest(testSuite, "GetListForWrongType", _testGetListForWrongType);
  addTest(testSuite, "GetListForInvalidOption", _testGetListForInvalidOption);
  addTest(testSuite, "SetString", _testSetString);
  addTest(testSuite, "SetStringForWrongType", _testSetStringForWrongType);
  addTest(testSuite, "SetStringForInvalidOption",
          _testSetStringForInvalidOption);
  addTest(testSuite, "SetStringWithNull", _testSetStringWithNull);
  addTest(testSuite, "SetNumeric", _testSetNumeric);
  addTest(testSuite, "SetNumericForWrongType", _testSetNumericForWrongType);
  addTest(testSuite, "SetNumericForInvalidOption",
          _testSetNumericForInvalidOption);
  addTest(testSuite, "SetListItem", _testSetListItem);
  addTest(testSuite, "SetListItemForWrongType", _testSetListItemForWrongType);
  addTest(testSuite, "SetListItemForInvalidOption",
          _testSetListItemForInvalidOption);

  return testSuite;
}
