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

#include "renderer/native_render_impl.h"
#include "oh_napi/oh_napi_task_runner.h"

namespace hippy {
inline namespace render {
inline namespace native {

NativeRenderImpl::NativeRenderImpl(uint32_t instance_id) : instance_id_(instance_id) {}

void NativeRenderImpl::InitRenderManager() {
  auto native_render = std::static_pointer_cast<NativeRender>(shared_from_this());
  hr_manager_ = std::make_shared<HRManager>(instance_id_, native_render);
}

void NativeRenderImpl::RegisterNativeXComponentHandle(OH_NativeXComponent *nativeXComponent, uint32_t root_id) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }
  
  view_manager->AttachToNativeXComponent(nativeXComponent);
}

void NativeRenderImpl::CreateNode(uint32_t root_id, const std::vector<std::shared_ptr<HRCreateMutation>> &mutations) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  auto virtual_view_manager = hr_manager_->GetVirtualNodeManager(root_id);
  if (!view_manager || !virtual_view_manager) {
    return;
  }

  for (uint32_t i = 0; i < mutations.size(); i++) {
    auto &m = mutations[i];

    auto virtual_node = virtual_view_manager->CreateVirtualNode(root_id, m->tag_, m->parent_tag_, m->index_, m->props_);
    virtual_node->view_name_ = m->view_name_;
    virtual_view_manager->AddVirtualNode(m->tag_, virtual_node);

    auto tm = std::static_pointer_cast<HRMutation>(m);
    view_manager->AddMutations(tm);
  }
}

void NativeRenderImpl::UpdateNode(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateMutation>> &mutations) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  auto virtual_view_manager = hr_manager_->GetVirtualNodeManager(root_id);
  if (!view_manager || !virtual_view_manager) {
    return;
  }

  for (uint32_t i = 0; i < mutations.size(); i++) {
    auto &m = mutations[i];

    auto virtual_node = virtual_view_manager->GetVirtualNode(m->tag_);
    if (virtual_node && virtual_node->props_.size() > 0) {
      // TODO(hot):
    }

    auto tm = std::static_pointer_cast<HRMutation>(m);
    view_manager->AddMutations(tm);
  }
}

void NativeRenderImpl::MoveNode(uint32_t root_id, const std::shared_ptr<HRMoveMutation> &mutation) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }

  auto tm = std::static_pointer_cast<HRMutation>(mutation);
  view_manager->AddMutations(tm);
}

void NativeRenderImpl::MoveNode2(uint32_t root_id, const std::shared_ptr<HRMove2Mutation> &mutation) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }

  auto tm = std::static_pointer_cast<HRMutation>(mutation);
  view_manager->AddMutations(tm);
}

void NativeRenderImpl::DeleteNode(uint32_t root_id, const std::vector<std::shared_ptr<HRDeleteMutation>> &mutations) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }
  
  for (uint32_t i = 0; i < mutations.size(); i++) {
    auto &m = mutations[i];
    auto tm = std::static_pointer_cast<HRMutation>(m);
    view_manager->AddMutations(tm);
  }
}

void NativeRenderImpl::UpdateLayout(uint32_t root_id, const std::vector<std::shared_ptr<HRUpdateLayoutMutation>> &mutations) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }

  for (uint32_t i = 0; i < mutations.size(); i++) {
    auto &m = mutations[i];
    auto tm = std::static_pointer_cast<HRMutation>(m);
    view_manager->AddMutations(tm);
  }
}

void NativeRenderImpl::UpdateEventListener(uint32_t root_id,
                         const std::vector<std::shared_ptr<HRUpdateEventListenerMutation>> &mutations) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }

  for (uint32_t i = 0; i < mutations.size(); i++) {
    auto &m = mutations[i];
    auto tm = std::static_pointer_cast<HRMutation>(m);
    view_manager->AddMutations(tm);
  }
}

void NativeRenderImpl::EndBatch(uint32_t root_id) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }

  view_manager->ApplyMutations();
  view_manager->NotifyEndBatchCallbacks();
}

bool NativeRenderImpl::CheckRegisteredEvent(uint32_t root_id, uint32_t node_id, std::string &event_name) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return false;
  }
  return view_manager->CheckRegisteredEvent(node_id, event_name);
}

uint64_t NativeRenderImpl::AddEndBatchCallback(uint32_t root_id, const EndBatchCallback &cb) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return 0;
  }
  return view_manager->AddEndBatchCallback(cb);
}

void NativeRenderImpl::RemoveEndBatchCallback(uint32_t root_id, uint64_t cbId) {
  auto view_manager = hr_manager_->GetViewManager(root_id);
  if (!view_manager) {
    return;
  }
  view_manager->RemoveEndBatchCallback(cbId);
}

} // namespace native
} // namespace render
} // namespace hippy