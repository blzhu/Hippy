/*
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#pragma once

#include <js_native_api.h>
#include <js_native_api_types.h>
#include <memory>
#include "renderer/native_render_impl.h"
#include "renderer/uimanager/hr_mutation.h"
#include "renderer/utils/hr_event_utils.h"

namespace hippy {
inline namespace render {
inline namespace native {

class NativeRenderProvider {
public:
  NativeRenderProvider(uint32_t instance_id);
  ~NativeRenderProvider() = default;
  
  uint32_t GetInstanceId() { return instance_id_; }
  
  std::shared_ptr<NativeRenderImpl> &GetNativeRenderImpl() { return render_impl_; }
  
  void SetTsEnv(napi_env ts_env) { ts_env_ = ts_env; }

  void RegisterNativeXComponentHandle(OH_NativeXComponent *nativeXComponent, uint32_t root_id);

  void CreateNode(uint32_t root_id, const std::vector<std::shared_ptr<HRCreateMutation>> &mutations);
  void UpdateNode(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateMutation>> &mutations);
  void MoveNode(uint32_t root_id, const std::shared_ptr<HRMoveMutation> &mutation);
  void MoveNode2(uint32_t root_id, const std::shared_ptr<HRMove2Mutation> &mutation);
  void DeleteNode(uint32_t root_id, const std::vector<std::shared_ptr<HRDeleteMutation>> &mutations);
  void UpdateLayout(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateLayoutMutation>> &mutations);
  void UpdateEventListener(uint32_t root_id,
                           const std::vector<std::shared_ptr<HRUpdateEventListenerMutation>> &mutations);
  void EndBatch(uint32_t root_id);
  
  void CallUIFunction(uint32_t root_id, uint32_t node_id, uint32_t cb_id, std::string &func_name, const std::vector<HippyValue> params);

  void OnSize(uint32_t root_id, float width, float height);
  void OnSize2(uint32_t root_id, uint32_t node_id, float width, float height, bool isSync);
  void DispatchEvent(uint32_t root_id, uint32_t node_id, const std::string &event_name,
      const std::shared_ptr<HippyValue> &params, bool capture, bool bubble, HREventType event_type);
  void DoCallBack(int32_t result, uint32_t cb_id, std::string &func_name,
      uint32_t root_id, uint32_t node_id, std::shared_ptr<HippyValue> &params);
  
private:
  constexpr static const char * EVENT_PREFIX = "on";
  
  napi_env ts_env_ = 0;
  uint32_t instance_id_;
  std::shared_ptr<NativeRenderImpl> render_impl_;
};

} // namespace native
} // namespace render
} // namespace hippy