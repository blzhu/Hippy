/* Tencent is pleased to support the open source community by making Hippy available.
 * Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.tencent.mtt.hippy.uimanager;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.tencent.mtt.hippy.annotation.HippyControllerProps;
import com.tencent.mtt.hippy.dom.node.NodeProps;

import com.tencent.mtt.hippy.views.custom.HippyCustomPropsController;
import com.tencent.renderer.Renderer;
import com.tencent.renderer.component.Component;
import com.tencent.renderer.component.ComponentController;
import com.tencent.renderer.component.image.ImageComponentController;
import com.tencent.renderer.node.TextRenderNode;
import com.tencent.renderer.node.TextVirtualNode;
import com.tencent.renderer.utils.PropertyUtils;
import com.tencent.renderer.utils.PropertyUtils.PropertyMethodHolder;
import com.tencent.renderer.node.RenderNode;

import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class ControllerUpdateManger<T, G> {

    private static final Map<Class<?>, Map<String, PropertyMethodHolder>> sViewPropsMethodMap = new HashMap<>();
    private static final Map<String, PropertyMethodHolder> sComponentPropsMethodMap = new HashMap<>();
    private static final Set<String> sTextPropsMap = new HashSet<>();
    @NonNull
    private final Renderer mRenderer;
    @Nullable
    private ComponentController mComponentController;
    @Nullable
    private ImageComponentController mImageComponentController;
    @Nullable
    private T mCustomPropsController;

    public ControllerUpdateManger(@NonNull Renderer renderer) {
        mRenderer = renderer;
        initPropsMap();
    }

    public void setCustomPropsController(T controller) {
        mCustomPropsController = controller;
    }

    private void collectMethodHolder(@NonNull Class<?> cls,
            @NonNull Map<String, PropertyMethodHolder> methodHolderMap) {
        Method[] methods = cls.getMethods();
        for (Method method : methods) {
            HippyControllerProps controllerProps = method
                    .getAnnotation(HippyControllerProps.class);
            if (controllerProps != null) {
                String style = controllerProps.name();
                PropertyMethodHolder propsMethodHolder = new PropertyMethodHolder();
                propsMethodHolder.defaultNumber = controllerProps.defaultNumber();
                propsMethodHolder.defaultType = controllerProps.defaultType();
                propsMethodHolder.defaultString = controllerProps.defaultString();
                propsMethodHolder.defaultBoolean = controllerProps.defaultBoolean();
                propsMethodHolder.method = method;
                propsMethodHolder.hostClass = cls;
                methodHolderMap.put(style, propsMethodHolder);
            }
        }
    }

    public boolean checkComponentProperty(@NonNull String key) {
        return sComponentPropsMethodMap.containsKey(key);
    }

    private void initPropsMap() {
        collectMethodHolder(ComponentController.class, sComponentPropsMethodMap);
        collectMethodHolder(ImageComponentController.class, sComponentPropsMethodMap);
        Method[] methods = TextVirtualNode.class.getMethods();
        for (Method method : methods) {
            HippyControllerProps controllerProps = method
                    .getAnnotation(HippyControllerProps.class);
            if (controllerProps != null) {
                sTextPropsMap.add(controllerProps.name());
            }
        }
    }

    void findViewPropsMethod(Class<?> cls,
            @NonNull Map<String, PropertyMethodHolder> methodHolderMap) {
        if (cls != HippyViewController.class) {
            // find parent methods first
            findViewPropsMethod(cls.getSuperclass(), methodHolderMap);
        }
        Map<String, PropertyMethodHolder> methodHolder = sViewPropsMethodMap.get(cls);
        if (methodHolder == null) {
            collectMethodHolder(cls, methodHolderMap);
            sViewPropsMethodMap.put(cls, new HashMap<>(methodHolderMap));
        } else {
            methodHolderMap.putAll(methodHolder);
        }
    }

    private void invokePropMethod(@NonNull Object obj, @NonNull Object arg1,
            Map<String, Object> props, String key, @NonNull PropertyMethodHolder methodHolder) {
        try {
            if (props.get(key) == null) {
                switch (methodHolder.defaultType) {
                    case HippyControllerProps.BOOLEAN:
                        methodHolder.method.invoke(obj, arg1, methodHolder.defaultBoolean);
                        break;
                    case HippyControllerProps.NUMBER:
                        if (methodHolder.paramTypes == null) {
                            methodHolder.paramTypes = methodHolder.method
                                    .getGenericParameterTypes();
                        }
                        methodHolder.method.invoke(obj, arg1, PropertyUtils
                                .convertNumber(methodHolder.paramTypes[1],
                                        methodHolder.defaultNumber));
                        break;
                    case HippyControllerProps.STRING:
                        methodHolder.method.invoke(obj, arg1, methodHolder.defaultString);
                        break;
                    default:
                        methodHolder.method.invoke(obj, arg1, null);
                        break;
                }
            } else {
                Object value = props.get(key);
                if (methodHolder.paramTypes == null) {
                    methodHolder.paramTypes = methodHolder.method.getGenericParameterTypes();
                }
                value = PropertyUtils.convertProperty(methodHolder.paramTypes[1], value);
                methodHolder.method.invoke(obj, arg1, value);
            }
        } catch (Exception exception) {
            mRenderer.handleRenderException(
                    PropertyUtils.makePropertyConvertException(exception, key,
                            methodHolder.method));
        }
    }

    private void handleCustomProps(T t, G g, @NonNull String key,
            @NonNull Map<String, Object> props) {
        boolean hasCustomMethodHolder = false;
        if (!(g instanceof View)) {
            return;
        }
        Object customProps = props.get(key);
        if (mCustomPropsController != null
                && mCustomPropsController instanceof HippyCustomPropsController) {
            Class<?> cls = mCustomPropsController.getClass();
            Map<String, PropertyMethodHolder> methodHolderMap = sViewPropsMethodMap.get(cls);
            if (methodHolderMap == null) {
                methodHolderMap = new HashMap<>();
                findViewPropsMethod(cls, methodHolderMap);
            }
            PropertyMethodHolder methodHolder = methodHolderMap.get(key);
            if (methodHolder != null) {
                invokePropMethod(mCustomPropsController, g, props, key, methodHolder);
                hasCustomMethodHolder = true;
            }
        }
        if (!hasCustomMethodHolder && t instanceof HippyViewController) {
            //noinspection unchecked
            ((HippyViewController) t).setCustomProp((View) g, key, customProps);
        }
    }

    protected void updateProps(@NonNull RenderNode node, @NonNull T controller, @Nullable G view,
            @Nullable Map<String, Object> props, boolean skipComponentProps) {
        if (props == null || props.isEmpty()) {
            return;
        }
        Class<?> cls = controller.getClass();
        Map<String, PropertyMethodHolder> methodHolderMap = sViewPropsMethodMap.get(cls);
        if (methodHolderMap == null) {
            methodHolderMap = new HashMap<>();
            findViewPropsMethod(cls, methodHolderMap);
        }
        Set<String> keySet = props.keySet();
        for (String key : keySet) {
            if (node instanceof TextRenderNode && sTextPropsMap.contains(key)) {
                // The text related attributes have been processed in the build layout,
                // so the following process no longer needs to be executed.
                continue;
            }
            PropertyMethodHolder methodHolder = methodHolderMap.get(key);
            if (methodHolder != null) {
                Object arg = (view == null) ? node.createView(true) : view;
                if (arg != null) {
                    invokePropMethod(controller, arg, props, key, methodHolder);
                }
            } else {
                if (key.equals(NodeProps.STYLE) && props.get(key) instanceof Map) {
                    updateProps(node, controller, view, (Map) props.get(key), skipComponentProps);
                } else if (!handleComponentProps(node, key, props, skipComponentProps)) {
                    handleCustomProps(controller, view, key, props);
                }
            }
        }
    }

    @Nullable
    private Object getComponentController(Class<?> cls) {
        if (cls == ComponentController.class) {
            if (mComponentController == null) {
                mComponentController = new ComponentController();
            }
            return mComponentController;
        }
        if (cls == ImageComponentController.class) {
            if (mImageComponentController == null) {
                mImageComponentController = new ImageComponentController();
            }
            return mImageComponentController;
        }
        return null;
    }

    private boolean handleComponentProps(@NonNull RenderNode node, @NonNull String key,
            @NonNull Map<String, Object> props, boolean skipComponentProps) {
        PropertyMethodHolder methodHolder = sComponentPropsMethodMap.get(key);
        if (methodHolder != null) {
            if (!skipComponentProps) {
                Object controller = getComponentController(methodHolder.hostClass);
                Component component = node.ensureComponentIfNeeded(methodHolder.hostClass);
                if (controller == null || component == null) {
                    return false;
                }
                invokePropMethod(controller, component, props, key, methodHolder);
            }
            return true;
        }
        return false;
    }
}
