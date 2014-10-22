//===- BuildEngine.h --------------------------------------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef LLBUILD_CORE_BUILDENGINE_H
#define LLBUILD_CORE_BUILDENGINE_H

#include <functional>
#include <string>
#include <utility>

namespace llbuild {
namespace core {

// FIXME: Need to abstract KeyType;
typedef std::string KeyType;
// FIXME: Need to abstract ValueType;
typedef int ValueType;

class BuildEngine;

/// A task object represents an abstract in progress computation in the build
/// engine.
///
/// The task represents not just the primary computation, but also the process
/// of starting the computation and necessary input dependencies. Tasks are
/// expected to be created in response to \see BuildEngine requests to initiate
/// the production of particular result value.
///
/// The creator may use \see BuildEngine::taskNeedsInput() to specify input
/// dependencies on the Task. The Task itself may also specify additional input
/// dependencies dynamically during the execution of \see Task::start() or \see
/// Task::provideValue().
///
/// Once a task has been created and registered, the BuildEngine will invoke
/// \see Task::start() to initiate the computation. The BuildEngine will provide
/// the in progress task with its requested inputs via \see
/// Task::provideValue().
///
/// After all inputs requested by the Task have been delivered, the BuildEngine
/// will invoke \see Task::finish() to instruct the Task to complete its
/// computation and provide the output.
//
// FIXME: Define parallel execution semantics.
class Task {
public:
  /// The name of the task, for debugging purposes.
  //
  // FIXME: Eliminate this?
  std::string Name;

public:
  Task(const std::string& Name) : Name(Name) {}

  virtual ~Task();

  /// Executed by the build engine when the task should be started.
  virtual void start(BuildEngine&) = 0;

  /// Invoked by the build engine to provide an input value as it becomes
  /// available.
  ///
  /// \param InputID The unique identifier provided to the build engine to
  /// represent this input when requested in \see
  /// BuildEngine::taskNeedsInput().
  ///
  /// \param Value The computed value for the given input.
  virtual void provideValue(BuildEngine&, uintptr_t InputID,
                             ValueType Value) = 0;

  /// Executed by the build engine to retrieve the task output, after all inputs
  /// have been provided.
  //
  // FIXME: Is it ever useful to provide the build engine here? It would be more
  // symmetric.
  virtual ValueType finish() = 0;
};

class Rule {
public:
  KeyType Key;
  std::function<Task*(BuildEngine&)> Action;
};

class BuildEngine {
  void *Impl;

public:
  /// Create a build engine using a particular database delegate.
  explicit BuildEngine();
  ~BuildEngine();

  /// @name Rule Definition
  /// @{

  /// Add a rule which the engine can use to produce outputs.
  void addRule(Rule &&Rule);

  /// @}

  /// @name Client API
  /// @{

  /// Build the result for a particular key.
  ValueType build(KeyType Key);

  /// Enable tracing into the given output file.
  ///
  /// \returns True on success.
  bool enableTracing(const std::string& Path, std::string* Error_Out);

  /// @}

  /// @name Task Management APIs
  /// @{

  /// Register the given task, in response to a Rule evaluation.
  ///
  /// The engine tasks ownership of the \arg Task, and it is expected to
  /// subsequently be returned as the task to execute for a Rule evaluation.
  ///
  /// \returns The provided task, for the convenience of the client.
  Task* registerTask(Task* Task);

  /// Specify the given \arg Task depends upon the result of computing \arg Key.
  ///
  /// The result, when available, will be provided to the task via \see
  /// Task::provideValue(), supplying the provided \arg InputID to allow the
  /// task to identify the particular input.
  void taskNeedsInput(Task* Task, KeyType Key, uintptr_t InputID);
  
  /// @}
};

}
}

#endif
