//
// ProgramOption.c - MrsWatson
// Created by Nik Reiman on 1/2/12.
// Copyright (c) 2012 Teragon Audio. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/ProgramOption.h"
#include "base/FileUtilities.h"
#include "base/StringUtilities.h"
#include "logging/EventLogger.h"
#include "sequencer/AudioSettings.h"

ProgramOption newProgramOption(void) {
  return newProgramOptionWithValues(-1, "", "", false, ARGUMENT_TYPE_INVALID, -1);
}

ProgramOption newProgramOptionWithValues(const int optionIndex, const char* name,
  const char* help, boolByte hasShortForm, ProgramOptionArgumentType argumentType,
  int defaultValue) {
  ProgramOption option = (ProgramOption)malloc(sizeof(ProgramOptionMembers));

  option->index = optionIndex;
  option->name = newCharStringWithCString(name);
  option->help = newCharStringWithCString(help);
  option->helpDefaultValue = defaultValue;
  option->hasShortForm = hasShortForm;

  option->argumentType = argumentType;
  option->argument = newCharString();
  option->enabled = false;

  return option;
}

void programOptionPrintHelp(const ProgramOption self, boolByte withFullHelp, int indentSize, int initialIndent) {
  CharString wrappedHelpString;
  int i;

  if(self == NULL) {
    logError("Can't find help for that option. Try running with --help to see all options\n");
    return;
  }

  // Initial argument indent
  for(i = 0; i < initialIndent; i ++) {
    printf(" ");
  }

  // All arguments have a long form, so that will always be printed
  printf("--%s", self->name->data);

  if(self->hasShortForm) {
    printf(" (or -%c)", self->name->data[0]);
  }

  switch(self->argumentType) {
    case ARGUMENT_TYPE_REQUIRED:
      printf(" <argument>");
      break;
    case ARGUMENT_TYPE_OPTIONAL:
      printf(" [argument]");
      break;
    case ARGUMENT_TYPE_NONE:
    default:
      break;
  }

  if(self->helpDefaultValue != NO_DEFAULT_VALUE) {
    printf(", default value: %d", self->helpDefaultValue);
  }

  if(withFullHelp) {
    // Newline and indentation before help
    wrappedHelpString = newCharStringWithCapacity(STRING_LENGTH_LONG);
    wrapString(self->help->data, wrappedHelpString->data, initialIndent + indentSize);
    printf("\n%s\n\n", wrappedHelpString->data);
    freeCharString(wrappedHelpString);
  }
  else {
    printf("\n");
  }
}

void freeProgramOption(ProgramOption programOption) {
  freeCharString(programOption->name);
  freeCharString(programOption->help);
  freeCharString(programOption->argument);
  free(programOption);
}


ProgramOptions newProgramOptions(int numOptions) {
  ProgramOptions options = (ProgramOptions)malloc(sizeof(ProgramOptionsMembers));
  options->numOptions = numOptions;
  options->options = (ProgramOption*)malloc(sizeof(ProgramOption) * numOptions);
  return options;
}

void programOptionsAdd(const ProgramOptions self, const ProgramOption option) {
  self->options[option->index] = option;
}

static boolByte _isStringShortOption(const char* testString) {
  return (boolByte)(testString != NULL && strlen(testString) == 2 && testString[0] == '-');
}

static boolByte _isStringLongOption(const char* testString) {
  return (boolByte)(testString != NULL && strlen(testString) > 2 && testString[0] == '-' && testString[1] == '-');
}

static ProgramOption _findProgramOption(ProgramOptions self, const char* name) {
  ProgramOption potentialMatchOption, optionMatch;
  CharString optionStringWithoutDashes;
  int i;

  if(_isStringShortOption(name)) {
    for(i = 0; i < self->numOptions; i++) {
      potentialMatchOption = self->options[i];
      if(potentialMatchOption->hasShortForm && potentialMatchOption->name->data[0] == name[1]) {
        return potentialMatchOption;
      }
    }
  }

  if(_isStringLongOption(name)) {
    optionMatch = NULL;
    optionStringWithoutDashes = newCharStringWithCapacity(STRING_LENGTH_SHORT);
    strncpy(optionStringWithoutDashes->data, name + 2, strlen(name) - 2);
    for(i = 0; i < self->numOptions; i++) {
      potentialMatchOption = self->options[i];
      if(charStringIsEqualTo(potentialMatchOption->name, optionStringWithoutDashes, false)) {
        optionMatch = potentialMatchOption;
        break;
      }
    }
    freeCharString(optionStringWithoutDashes);
    return optionMatch;
  }

  // If no option was found, then return null
  return NULL;
}

static boolByte _fillOptionArgument(ProgramOption self, int* currentArgc, int argc, char** argv) {
  if(self->argumentType == ARGUMENT_TYPE_NONE) {
    return true;
  }
  else if(self->argumentType == ARGUMENT_TYPE_OPTIONAL) {
    int potentialNextArgc = *currentArgc + 1;
    if(potentialNextArgc >= argc) {
      return true;
    }
    else {
      char* potentialNextArg = argv[potentialNextArgc];
      // If the next string in the sequence is NOT an argument, we assume it is the optional argument
      if(!_isStringShortOption(potentialNextArg) && !_isStringLongOption(potentialNextArg)) {
        charStringCopyCString(self->argument, potentialNextArg);
        (*currentArgc)++;
        return true;
      }
      else {
        // Otherwise, it is another option, but that's ok
        return true;
      }
    }
  }
  else if(self->argumentType == ARGUMENT_TYPE_REQUIRED) {
    int nextArgc = *currentArgc + 1;
    if(nextArgc >= argc) {
      logCritical("Option '%s' requires an argument, but none was given", self->name->data);
      return false;
    }
    else {
      char* nextArg = argv[nextArgc];
      if(_isStringShortOption(nextArg) || _isStringLongOption(nextArg)) {
        logCritical("Option '%s' requires an argument, but '%s' is not valid", self->name->data, nextArg);
        return false;
      }
      else {
        charStringCopyCString(self->argument, nextArg);
        (*currentArgc)++;
        return true;
      }
    }
  }
  else {
    logInternalError("Unknown argument type '%d'", self->argumentType);
    return false;
  }
}

boolByte programOptionsParseArgs(ProgramOptions self, int argc, char** argv) {
  int argumentIndex;
  for(argumentIndex = 1; argumentIndex < argc; argumentIndex++) {
    const ProgramOption option = _findProgramOption(self, argv[argumentIndex]);
    if(option == NULL) {
      logCritical("Invalid option '%s'", argv[argumentIndex]);
      return false;
    }
    else {
      option->enabled = true;
      if(!_fillOptionArgument(option, &argumentIndex, argc, argv)) {
        return false;
      }
    }
  }

  // If we make it to here, return true
  return true;
}

void programOptionsPrintHelp(const ProgramOptions self, boolByte withFullHelp, int indentSize) {
  int i;
  for(i = 0; i < self->numOptions; i++) {
    programOptionPrintHelp(self->options[i], withFullHelp, indentSize, indentSize);
  }
}

ProgramOption programOptionsFind(const ProgramOptions self, const CharString string) {
  int i;
  for(i = 0; i < self->numOptions; i++) {
    if(charStringIsEqualTo(string, self->options[i]->name, true)) {
      return self->options[i];
    }
  }
  return NULL;
}


void freeProgramOptions(ProgramOptions programOptions) {
  ProgramOption option;
  int i;

  for(i = 0; i < programOptions->numOptions; i++) {
    option = programOptions->options[i];
    freeProgramOption(option);
  }
  free(programOptions->options);
  free(programOptions);
}