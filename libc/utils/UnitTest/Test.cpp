//===--------- Implementation of the base class for libc unittests --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Test.h"

#include "utils/testutils/ExecuteFunction.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

namespace __llvm_libc {
namespace testing {

// This need not be a class as all it has is a single read-write state variable.
// But, we make it class as then its implementation can be hidden from the
// header file.
class RunContext {
public:
  enum RunResult { Result_Pass = 1, Result_Fail = 2 };

  RunResult status() const { return Status; }

  void markFail() { Status = Result_Fail; }

private:
  RunResult Status = Result_Pass;
};

namespace internal {

template <typename ValType>
bool test(RunContext &Ctx, TestCondition Cond, ValType LHS, ValType RHS,
          const char *LHSStr, const char *RHSStr, const char *File,
          unsigned long Line) {
  switch (Cond) {
  case Cond_EQ:
    if (LHS == RHS)
      return true;

    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "      Expected: " << LHSStr << '\n'
                 << "      Which is: " << LHS << '\n'
                 << "To be equal to: " << RHSStr << '\n'
                 << "      Which is: " << RHS << '\n';

    return false;
  case Cond_NE:
    if (LHS != RHS)
      return true;

    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "          Expected: " << LHSStr << '\n'
                 << "          Which is: " << LHS << '\n'
                 << "To be not equal to: " << RHSStr << '\n'
                 << "          Which is: " << RHS << '\n';
    return false;
  case Cond_LT:
    if (LHS < RHS)
      return true;

    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "       Expected: " << LHSStr << '\n'
                 << "       Which is: " << LHS << '\n'
                 << "To be less than: " << RHSStr << '\n'
                 << "       Which is: " << RHS << '\n';
    return false;
  case Cond_LE:
    if (LHS <= RHS)
      return true;

    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "                   Expected: " << LHSStr << '\n'
                 << "                   Which is: " << LHS << '\n'
                 << "To be less than or equal to: " << RHSStr << '\n'
                 << "                   Which is: " << RHS << '\n';
    return false;
  case Cond_GT:
    if (LHS > RHS)
      return true;

    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "          Expected: " << LHSStr << '\n'
                 << "          Which is: " << LHS << '\n'
                 << "To be greater than: " << RHSStr << '\n'
                 << "          Which is: " << RHS << '\n';
    return false;
  case Cond_GE:
    if (LHS >= RHS)
      return true;

    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "                      Expected: " << LHSStr << '\n'
                 << "                      Which is: " << LHS << '\n'
                 << "To be greater than or equal to: " << RHSStr << '\n'
                 << "                      Which is: " << RHS << '\n';
    return false;
  default:
    Ctx.markFail();
    llvm::outs() << "Unexpected test condition.\n";
    return false;
  }
}

} // namespace internal

Test *Test::Start = nullptr;
Test *Test::End = nullptr;

void Test::addTest(Test *T) {
  if (End == nullptr) {
    Start = T;
    End = T;
    return;
  }

  End->Next = T;
  End = T;
}

int Test::runTests() {
  int TestCount = 0;
  int FailCount = 0;
  for (Test *T = Start; T != nullptr; T = T->Next, ++TestCount) {
    const char *TestName = T->getName();
    constexpr auto GREEN = llvm::raw_ostream::GREEN;
    constexpr auto RED = llvm::raw_ostream::RED;
    constexpr auto RESET = llvm::raw_ostream::RESET;
    llvm::outs() << GREEN << "[ RUN      ] " << RESET << TestName << '\n';
    RunContext Ctx;
    T->SetUp();
    T->Run(Ctx);
    T->TearDown();
    auto Result = Ctx.status();
    switch (Result) {
    case RunContext::Result_Fail:
      llvm::outs() << RED << "[  FAILED  ] " << RESET << TestName << '\n';
      ++FailCount;
      break;
    case RunContext::Result_Pass:
      llvm::outs() << GREEN << "[       OK ] " << RESET << TestName << '\n';
      break;
    }
  }

  llvm::outs() << "Ran " << TestCount << " tests. "
               << " PASS: " << TestCount - FailCount << ' '
               << " FAIL: " << FailCount << '\n';

  return FailCount > 0 ? 1 : 0;
}

template bool Test::test<char, 0>(RunContext &Ctx, TestCondition Cond, char LHS,
                                  char RHS, const char *LHSStr,
                                  const char *RHSStr, const char *File,
                                  unsigned long Line);

template bool Test::test<short, 0>(RunContext &Ctx, TestCondition Cond,
                                   short LHS, short RHS, const char *LHSStr,
                                   const char *RHSStr, const char *File,
                                   unsigned long Line);

template bool Test::test<int, 0>(RunContext &Ctx, TestCondition Cond, int LHS,
                                 int RHS, const char *LHSStr,
                                 const char *RHSStr, const char *File,
                                 unsigned long Line);

template bool Test::test<long, 0>(RunContext &Ctx, TestCondition Cond, long LHS,
                                  long RHS, const char *LHSStr,
                                  const char *RHSStr, const char *File,
                                  unsigned long Line);

template bool Test::test<long long, 0>(RunContext &Ctx, TestCondition Cond,
                                       long long LHS, long long RHS,
                                       const char *LHSStr, const char *RHSStr,
                                       const char *File, unsigned long Line);

template bool Test::test<unsigned char, 0>(RunContext &Ctx, TestCondition Cond,
                                           unsigned char LHS, unsigned char RHS,
                                           const char *LHSStr,
                                           const char *RHSStr, const char *File,
                                           unsigned long Line);

template bool
Test::test<unsigned short, 0>(RunContext &Ctx, TestCondition Cond,
                              unsigned short LHS, unsigned short RHS,
                              const char *LHSStr, const char *RHSStr,
                              const char *File, unsigned long Line);

template bool Test::test<unsigned int, 0>(RunContext &Ctx, TestCondition Cond,
                                          unsigned int LHS, unsigned int RHS,
                                          const char *LHSStr,
                                          const char *RHSStr, const char *File,
                                          unsigned long Line);

template bool Test::test<unsigned long, 0>(RunContext &Ctx, TestCondition Cond,
                                           unsigned long LHS, unsigned long RHS,
                                           const char *LHSStr,
                                           const char *RHSStr, const char *File,
                                           unsigned long Line);

template bool Test::test<bool, 0>(RunContext &Ctx, TestCondition Cond, bool LHS,
                                  bool RHS, const char *LHSStr,
                                  const char *RHSStr, const char *File,
                                  unsigned long Line);

template bool Test::test<unsigned long long, 0>(
    RunContext &Ctx, TestCondition Cond, unsigned long long LHS,
    unsigned long long RHS, const char *LHSStr, const char *RHSStr,
    const char *File, unsigned long Line);

bool Test::testStrEq(RunContext &Ctx, const char *LHS, const char *RHS,
                     const char *LHSStr, const char *RHSStr, const char *File,
                     unsigned long Line) {
  return internal::test(Ctx, Cond_EQ, llvm::StringRef(LHS),
                        llvm::StringRef(RHS), LHSStr, RHSStr, File, Line);
}

bool Test::testStrNe(RunContext &Ctx, const char *LHS, const char *RHS,
                     const char *LHSStr, const char *RHSStr, const char *File,
                     unsigned long Line) {
  return internal::test(Ctx, Cond_NE, llvm::StringRef(LHS),
                        llvm::StringRef(RHS), LHSStr, RHSStr, File, Line);
}

bool Test::testMatch(RunContext &Ctx, bool MatchResult, MatcherBase &Matcher,
                     const char *LHSStr, const char *RHSStr, const char *File,
                     unsigned long Line) {
  if (MatchResult)
    return true;

  Ctx.markFail();
  llvm::outs() << File << ":" << Line << ": FAILURE\n"
               << "Failed to match " << LHSStr << " against " << RHSStr
               << ".\n";
  testutils::StreamWrapper OutsWrapper = testutils::outs();
  Matcher.explainError(OutsWrapper);
  return false;
}

bool Test::testProcessKilled(RunContext &Ctx, testutils::FunctionCaller *Func,
                             int Signal, const char *LHSStr, const char *RHSStr,
                             const char *File, unsigned long Line) {
  testutils::ProcessStatus Result = testutils::invokeInSubprocess(Func, 500);

  if (const char *error = Result.getError()) {
    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n" << error << '\n';
    return false;
  }

  if (Result.timedOut()) {
    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "Process timed out after " << 500 << " milliseconds.\n";
    return false;
  }

  if (Result.exitedNormally()) {
    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "Expected " << LHSStr
                 << " to be killed by a signal\nBut it exited normally!\n";
    return false;
  }

  int KilledBy = Result.getFatalSignal();
  assert(KilledBy != 0 && "Not killed by any signal");
  if (Signal == -1 || KilledBy == Signal)
    return true;

  using testutils::signalAsString;
  Ctx.markFail();
  llvm::outs() << File << ":" << Line << ": FAILURE\n"
               << "              Expected: " << LHSStr << '\n'
               << "To be killed by signal: " << Signal << '\n'
               << "              Which is: " << signalAsString(Signal) << '\n'
               << "  But it was killed by: " << KilledBy << '\n'
               << "              Which is: " << signalAsString(KilledBy)
               << '\n';
  return false;
}

bool Test::testProcessExits(RunContext &Ctx, testutils::FunctionCaller *Func,
                            int ExitCode, const char *LHSStr,
                            const char *RHSStr, const char *File,
                            unsigned long Line) {
  testutils::ProcessStatus Result = testutils::invokeInSubprocess(Func, 500);

  if (const char *error = Result.getError()) {
    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n" << error << '\n';
    return false;
  }

  if (Result.timedOut()) {
    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "Process timed out after " << 500 << " milliseconds.\n";
    return false;
  }

  if (!Result.exitedNormally()) {
    Ctx.markFail();
    llvm::outs() << File << ":" << Line << ": FAILURE\n"
                 << "Expected " << LHSStr << '\n'
                 << "to exit with exit code " << ExitCode << '\n'
                 << "But it exited abnormally!\n";
    return false;
  }

  int ActualExit = Result.getExitCode();
  if (ActualExit == ExitCode)
    return true;

  Ctx.markFail();
  llvm::outs() << File << ":" << Line << ": FAILURE\n"
               << "Expected exit code of: " << LHSStr << '\n'
               << "             Which is: " << ActualExit << '\n'
               << "       To be equal to: " << RHSStr << '\n'
               << "             Which is: " << ExitCode << '\n';
  return false;
}

} // namespace testing
} // namespace __llvm_libc

int main() { return __llvm_libc::testing::Test::runTests(); }
