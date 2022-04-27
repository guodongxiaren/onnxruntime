// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "core/common/path.h"
#include "core/platform/env.h"
#include "core/platform/path_lib.h"
#include "core/common/path_string.h"
#include "core/common/status.h"
#include "core/framework/data_transfer_manager.h"
#include "core/framework/data_types.h"
#include "core/framework/framework_common.h"

namespace onnxruntime {
namespace training {

constexpr const PathChar* k_tensors_file_name = ORT_TSTR("tensors.pbseq");
constexpr const PathChar* k_tensors_data_file_name = ORT_TSTR("tensors.bin");
constexpr const PathChar* k_properties_file_name = ORT_TSTR("properties.pbseq");

PathString GetCheckpointTensorsFilePath(const PathString& checkpoint_directory, const std::string& filename_prefix = "");

PathString GetCheckpointTensorsDataFilePath(const PathString& checkpoint_directory, const std::string& filename_prefix = "");

PathString GetCheckpointPropertiesFilePath(const PathString& checkpoint_directory, const std::string& filename_prefix = "");

/**
 * A checkpoint is a directory of files:
 * checkpoint/
 *   tensors.pbseq - tensor protobuf messages
 *   tensors.bin - tensor binary data
 *   properties.pbseq - property protobuf messages
 */

/**
 * Saves a model checkpoint in the specified location.
 *
 * @param checkpoint_path The checkpoint location.
 * @param data_transfer_manager The DataTransferManager instance.
 * @param runtime_tensors The tensors to persist.
 * @param properties The properties to persist.
 * @return The status of the operation.
 */
common::Status SaveModelCheckpoint(
    const PathString& checkpoint_path,
    const DataTransferManager& data_transfer_manager,
    const NameMLValMap& runtime_tensors,
    const std::unordered_map<std::string, std::string>& properties);

// opens file descriptor and calls use_fn
//   use_fn should have this signature: Status use_fn(int file_descriptor)
template <typename TUseFileFn>
common::Status WithOpenFile(const PathString& path, bool readonly, TUseFileFn use_fn) {
  int fd;
  if (readonly) {
    ORT_RETURN_IF_ERROR(Env::Default().FileOpenRd(path, fd));
  } else {
    ORT_RETURN_IF_ERROR(Env::Default().FileOpenWr(path, fd));
  }

  Status use_fn_status{};
  try {
    use_fn_status = use_fn(fd);
  } catch (std::exception& e) {
    use_fn_status = ORT_MAKE_STATUS(ONNXRUNTIME, FAIL, e.what());
  }

  Status close_status = Env::Default().FileClose(fd);
  return !use_fn_status.IsOK() ? use_fn_status : close_status;
}

common::Status SaveRuntimeTensors(
    const PathString& tensors_path,
    const PathString& tensors_data_path,
    const DataTransferManager& data_transfer_manager,
    const NameMLValMap& ort_values,
    bool force_save_as_external_data = false);

/**
 * @brief Saves list of tensor proto in the specified location.
 *
 * @param proto_file_path the file path where stores the tensor protos.
 * @param tensor_protos_to_save tensor protos being saved.
 * @return The status of the operation.
 */
common::Status SaveTensorProtosToFile(
    const PathString& proto_file_path,
    const std::vector<ONNX_NAMESPACE::TensorProto>& tensor_protos_to_save);

/**
 * Loads a model checkpoint from the specified location.
 *
 * @param checkpoint_path The checkpoint location.
 * @param model_path The model location.
 * @param tensor_protos The loaded tensors.
 * @param properties The loaded properties.
 * @return The status of the operation.
 */
common::Status LoadModelCheckpoint(
    const PathString& checkpoint_path,
    const PathString& model_path,
    std::vector<ONNX_NAMESPACE::TensorProto>& tensor_protos,
    std::unordered_map<std::string, std::string>& properties);

}  // namespace training
}  // namespace onnxruntime
