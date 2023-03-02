
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <react/renderer/components/view/ViewProps.h>

namespace Microsoft::ReactNative {

interface IViewComponentDescriptor {
  ComponentHandle Handle();
}


extern const char MyComponentComponentName[];

class MyComponentDescriptor : IViewComponentDescriptor {

  ComponentHandle Handle() override { return ComponentHandle(MyComponentComponentName); }
  winrt::hstring Name() override { return winrt::to_hstring(MyComponentComponentName); }

  winrt::Microsoft::ReactNative::IViewProps CreateProps();
}

// Roughly needs to provide functionality that ConcreteViewShadowNode<> would but using ABI objects
class AbiViewShadowNode : YogaLayoutableShadowNode
{
  using BaseShadowNodeT = YogaLayoutableShadowNode;
  using PropsT = AbiViewProps;

  using BaseShadowNode = BaseShadowNodeT;

  using ConcreteProps = PropsT;
  using SharedConcreteProps = std::shared_ptr<PropsT const>;
  using UnsharedConcreteProps = std::shared_ptr<PropsT>;

  using EventEmitterT = ViewEventEmitter; // TODO event emitters
  using ConcreteEventEmitter = EventEmitterT;
  using SharedConcreteEventEmitter = std::shared_ptr<EventEmitterT const>;

  using ConcreteState = facebook::react::ConcreteState<facebook::react::StateData>; // TODO state
  using ConcreteStateData = facebook::react::StateData; // TODO state

  static ComponentName Name() {
    return m_iViewComponentDescriptor.Name();
  }

  static ComponentHandle Handle() {
    return m_iViewComponentDescriptor.Handle();
  }

  static UnsharedConcreteProps Props(
      const PropsParserContext &context,
      RawProps const &rawProps,
      Props::Shared const &baseProps = nullptr) {

    auto props = std::make_shared<PropsT>(
      m_iViewComponentDescriptor.CreateProps(),
        context,
        baseProps ? static_cast<PropsT const &>(*baseProps) : PropsT(),
        rawProps);
  }

  static SharedConcreteProps defaultSharedProps() {
    static const SharedConcreteProps defaultSharedProps =
        std::make_shared<const PropsT>(m_iViewComponentDescriptor.CreateProps());
    return defaultSharedProps;
  }

  // TODO state...
  static ConcreteStateData initialStateData(
      ShadowNodeFragment const &fragment,
      ShadowNodeFamilyFragment const &familyFragment,
      ComponentDescriptor const &componentDescriptor) {
    return {};
  }

  /*
   * Returns a concrete props object associated with the node.
   * Thread-safe after the node is sealed.
   */
  ConcreteProps const &getConcreteProps() const {
    react_native_assert(
        BaseShadowNodeT::props_ && "Props must not be `nullptr`.");
    return static_cast<ConcreteProps const &>(*props_);
  }

  /*
   * Returns a concrete event emitter object associated with the node.
   * Thread-safe after the node is sealed.
   */
  ConcreteEventEmitter const &getConcreteEventEmitter() const {
    return static_cast<ConcreteEventEmitter const &>(
        *BaseShadowNodeT::getEventEmitter());
  }

  /*
   * Returns a concrete state data associated with the node.
   * Thread-safe after the node is sealed.
   */
  ConcreteStateData const &getStateData() const {
    react_native_assert(state_ && "State must not be `nullptr`.");
    return static_cast<ConcreteState const *>(state_.get())->getData();
  }

  /*
   * Creates and assigns a new state object containing given state data.
   * Can be called only before the node is sealed (usually during construction).
   */
  void setStateData(ConcreteStateData &&data) {
    Sealable::ensureUnsealed();
    state_ = std::make_shared<ConcreteState const>(
        std::make_shared<ConcreteStateData const>(std::move(data)), *state_);
  }

  
  AbiViewShadowNode(
    IViewComponentDescriptor iViewComponentDescriptor,
      ShadowNodeFragment const &fragment,
      ShadowNodeFamily::Shared const &family,
      ShadowNodeTraits traits)
      : BaseShadowNode(fragment, family, traits),
      m_iViewComponentDescriptor(iViewComponentDescriptor) {
    initialize();
  }

  AbiViewShadowNode(
    IViewComponentDescriptor iViewComponentDescriptor,
      ShadowNode const &sourceShadowNode,
      ShadowNodeFragment const &fragment)
      : BaseShadowNode(sourceShadowNode, fragment) {
        m_iViewComponentDescriptor = sourceShadowNode.m_iViewComponentDescriptor
    initialize();
  }

  static ShadowNodeTraits BaseTraits() {
    auto traits = BaseShadowNode::BaseTraits();
    traits.set(ShadowNodeTraits::Trait::ViewKind);
    traits.set(ShadowNodeTraits::Trait::FormsStackingContext);
    traits.set(ShadowNodeTraits::Trait::FormsView);
    return traits;
  }

  Transform getTransform() const override {
    return BaseShadowNode::getConcreteProps().transform;
  }

#pragma mark - DebugStringConvertible

#if RN_DEBUG_STRING_CONVERTIBLE
  SharedDebugStringConvertibleList getDebugProps() const override {
    auto list = SharedDebugStringConvertibleList{};

    auto basePropsList = ShadowNode::getDebugProps();
    std::move(
        basePropsList.begin(), basePropsList.end(), std::back_inserter(list));

    list.push_back(std::make_shared<DebugStringConvertibleItem>(
        "layout", "", LayoutableShadowNode::getDebugProps()));

    return list;
  }
#endif

 private:
  void initialize() noexcept {
    auto &props = BaseShadowNode::getConcreteProps();

    if (props.yogaStyle.display() == YGDisplayNone) {
      BaseShadowNode::traits_.set(ShadowNodeTraits::Trait::Hidden);
    } else {
      BaseShadowNode::traits_.unset(ShadowNodeTraits::Trait::Hidden);
    }

    // `zIndex` is only defined for non-`static` positioned views.
    if (props.yogaStyle.positionType() != YGPositionTypeStatic) {
      BaseShadowNode::orderIndex_ = props.zIndex.value_or(0);
    } else {
      BaseShadowNode::orderIndex_ = 0;
    }
  }

  IViewComponentDescriptor m_iViewComponentDescriptor;

}

class AbiViewComponentDescriptor : facebook::react::ComponentDescriptor {
  
  AbiComponentDescriptor(IViewComponentDescriptor iViewComponentDescriptor, ComponentDescriptorParameters const &parameters) : facebook::react::ComponentDescriptor(parameters), m_iViewComponentDescriptor(iViewComponentDescriptor) {}
  virtual ~AbiComponentDescriptor() = default;

    /*
   * Returns `componentHandle` associated with particular kind of components.
   * All `ShadowNode`s of this type must return same `componentHandle`.
   */

// ComponentHandle is an int64, so it can be just a memory address of the component name or something. -- Needs to be same across all shadownodes and view descriptors of this component
  virtual ComponentHandle getComponentHandle() const { return m_iViewComponentDescriptor.Handle(); };

  /*
   * Returns component's name.
   * React uses a `name` to refer to particular kind of components in
   * `create` requests.
   */
  virtual ComponentName getComponentName() const { return m_iViewComponentDescriptor.Name(); };

  /*
   * Returns traits associated with a particular component type.
   */
  virtual ShadowNodeTraits getTraits() const { return AbiViewShadowNode::BaseTraits(); }

  /*
   * Creates a new `ShadowNode` of a particular component type.
   */
   // Diff from ConcreteDescriptor - extra param to AbiViewShadowNode ctor
  virtual ShadowNode::Shared createShadowNode(
      const ShadowNodeFragment &fragment,
      ShadowNodeFamily::Shared const &family) const {
            auto shadowNode =
        std::make_shared<AbiViewShadowNode>(m_iViewComponentDescriptor, fragment, family, getTraits());

    adopt(shadowNode);

    return shadowNode;

      }

  /*
   * Clones a `ShadowNode` with optionally new `props` and/or `children`.
   */
  virtual ShadowNode::Unshared cloneShadowNode(
      const ShadowNode &sourceShadowNode,
      const ShadowNodeFragment &fragment) const {
            auto shadowNode = std::make_shared<AbiViewShadowNode>(sourceShadowNode, fragment);

    adopt(shadowNode);
    return shadowNode;

      }

  /*
   * Appends (by mutating) a given `childShadowNode` to `parentShadowNode`.
   */
  virtual void appendChild(
      const ShadowNode::Shared &parentShadowNode,
      const ShadowNode::Shared &childShadowNode) const {
            auto concreteParentShadowNode =
        std::static_pointer_cast<const AbiViewShadowNode>(parentShadowNode);
    auto concreteNonConstParentShadowNode =
        std::const_pointer_cast<AbiViewShadowNode>(concreteParentShadowNode);
    concreteNonConstParentShadowNode->appendChild(childShadowNode);

      }

  /*
   * Creates a new `Props` of a particular type with all values copied from
   * `props` and `rawProps` applied on top of this.
   * If `props` is `nullptr`, a default `Props` object (with default values)
   * will be used.
   * Must return an object which is NOT pointer equal to `props`.
   */
  virtual Props::Shared cloneProps(
      const PropsParserContext &context,
      const Props::Shared &props,
      const RawProps &rawProps) const {
            // Optimization:
    // Quite often nodes are constructed with default/empty props: the base
    // `props` object is `null` (there no base because it's not cloning) and the
    // `rawProps` is empty. In this case, we can return the default props object
    // of a concrete type entirely bypassing parsing.
    if (!props && rawProps.isEmpty()) {
      return ShadowNodAbiViewShadowNodeeT::defaultSharedProps();
    }

    rawProps.parse(rawPropsParser_, context);

    // Call old-style constructor
    auto shadowNodeProps = AbiViewShadowNode::Props(context, rawProps, props);

    // Use the new-style iterator
    // Note that we just check if `Props` has this flag set, no matter
    // the type of ShadowNode; it acts as the single global flag.
    if (CoreFeatures::enablePropIteratorSetter) {
      rawProps.iterateOverValues([&](RawPropsPropNameHash hash,
                                     const char *propName,
                                     RawValue const &fn) {
        shadowNodeProps.get()->setProp(context, hash, propName, fn);
      });
    }

    return shadowNodeProps;
      }

  /*
   * Creates a new `Props` of a particular type with all values interpolated
   * between `props` and `newProps`.
   */
  virtual Props::Shared interpolateProps(
      const PropsParserContext &context,
      Float animationProgress,
      const Props::Shared &props,
      const Props::Shared &newProps) const = 0;

  /*
   * Create an initial State object that represents (and contains) an initial
   * State's data which can be constructed based on initial Props.
   */
  virtual State::Shared createInitialState(
      ShadowNodeFragment const &fragment,
      ShadowNodeFamily::Shared const &family) const = 0;

  /*
   * Creates a new State object that represents (and contains) a new version of
   * State's data.
   */
  virtual State::Shared createState(
      ShadowNodeFamily const &family,
      const StateData::Shared &data) const = 0;

  /*
   * Creates a shadow node family for particular node.
   */
  virtual ShadowNodeFamily::Shared createFamily(
      ShadowNodeFamilyFragment const &fragment,
      SharedEventTarget eventTarget) const = 0;

  private:
   IViewComponentDescriptor m_iViewComponentDescriptor;
}




} // namespace Microsoft::ReactNative