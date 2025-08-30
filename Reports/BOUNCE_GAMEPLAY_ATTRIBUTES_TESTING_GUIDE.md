# Bounce Gameplay Attributes Testing Guide

## Overview
This guide provides comprehensive testing procedures for the updated bounce implementation that uses Gameplay Attributes for state management, following Epic Games coding standards and GAS best practices.

## Testing Categories

### 1. Gameplay Attribute Integration Tests

#### Test 1.1: Attribute Initialization
**Objective**: Verify AirBounceCount attribute initializes correctly
**Steps**:
1. Start game with character
2. Open console and run: `showdebug abilitysystem`
3. Verify `AirBounceCount` shows as `0.0`

**Expected Result**: AirBounceCount = 0.0 on game start

#### Test 1.2: Attribute Increment
**Objective**: Test air bounce count increments via Gameplay Attributes
**Steps**:
1. Jump off platform to be airborne
2. Press bounce key once
3. Check attribute value in debug display
4. Press bounce key again
5. Check attribute value again

**Expected Result**: 
- After 1st air bounce: AirBounceCount = 1.0
- After 2nd air bounce: AirBounceCount = 2.0

#### Test 1.3: Attribute Reset on Landing
**Objective**: Verify attribute resets when character lands
**Steps**:
1. Perform air bounces until AirBounceCount > 0
2. Land on solid ground
3. Check attribute value after landing

**Expected Result**: AirBounceCount = 0.0 after landing

### 2. State Consistency Tests

#### Test 2.1: Validation/Execution Consistency
**Objective**: Ensure CanActivateAbility matches ExecuteBounce behavior
**Steps**:
1. Enable bounce debug logging: `bLogBounceEvents = true`
2. Perform air bounces until at limit (2/2)
3. Attempt another bounce
4. Verify logs show consistent validation

**Expected Logs**:
```
Bounce Validation: Grounded=false, AirBounces=2/2
Bounce Validation FAIL: Would exceed air bounce limit (2 would become 3/2)
```

#### Test 2.2: Ground/Air State Detection
**Objective**: Test accurate ground state detection
**Steps**:
1. Start on ground, press bounce
2. Jump and press bounce while ascending
3. Press bounce while falling
4. Land and immediately press bounce

**Expected Results**:
- Ground bounce: `"Ground bounce executed, air bounce count remains 0/2"`
- Air bounces: `"Air bounce executed, count now X/2"`

### 3. Epic Games Standards Compliance Tests

#### Test 3.1: Const Correctness
**Objective**: Verify all const methods work correctly
**Code Test**:
```cpp
// These should compile without error
const UGameplayAbility_Bounce* ConstBounce = GetBounceAbility();
int32 Count = ConstBounce->GetCurrentAirBounceCount();
bool CanBounce = ConstBounce->CanPerformAirBounce();
```

#### Test 3.2: RAII Resource Management
**Objective**: Verify proper cleanup of timers and resources
**Steps**:
1. Activate bounce ability
2. Force end ability early
3. Check that timers are properly cleared
4. Verify no memory leaks

#### Test 3.3: Blueprint Integration
**Objective**: Test Blueprint-accessible functions
**Steps**:
1. Create Blueprint node for `Get Air Bounce Count`
2. Create Blueprint node for `Reset Air Bounces`
3. Test functionality in Blueprint graphs

### 4. Performance and Maintainability Tests

#### Test 4.1: Network Replication
**Objective**: Verify AirBounceCount replicates correctly
**Steps**:
1. Enable network debugging
2. Test bounce in multiplayer scenario
3. Verify attribute synchronizes across clients

#### Test 4.2: Error Handling
**Objective**: Test graceful handling of invalid states
**Test Cases**:
- Invalid AbilitySystemComponent
- Missing AttributeSet
- Null character references

**Expected Behavior**: Graceful failure with appropriate logging, no crashes

### 5. Integration with Movement System

#### Test 5.1: Physics Integration
**Objective**: Verify bounce physics work correctly with attributes
**Steps**:
1. Perform ground bounce - check velocity applied
2. Perform air bounces - verify velocity reduction
3. Test horizontal momentum preservation

#### Test 5.2: Edge Cases
**Objective**: Test boundary conditions
**Test Cases**:
- Rapid bounce attempts
- Bouncing while movement is blocked
- Bouncing during other abilities
- Bouncing near geometry edges

## Automated Testing Code Examples

### Unit Test Example
```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBounceAttributeTest, "Game.Abilities.Bounce.AttributeManagement", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBounceAttributeTest::RunTest(const FString& Parameters)
{
    // Test attribute initialization
    UGameplayAbility_Bounce* BounceAbility = NewObject<UGameplayAbility_Bounce>();
    TestEqual("Initial air bounce count", BounceAbility->GetCurrentAirBounceCount(), 0);
    
    // Test increment
    BounceAbility->IncrementAirBounceCount();
    TestEqual("After increment", BounceAbility->GetCurrentAirBounceCount(), 1);
    
    // Test reset
    BounceAbility->ResetAirBounceCount();
    TestEqual("After reset", BounceAbility->GetCurrentAirBounceCount(), 0);
    
    return true;
}
```

### Console Commands for Testing
```cpp
// Add these console commands for easier testing
UFUNCTION(Exec)
void SetAirBounceCount(int32 Count);

UFUNCTION(Exec)  
void TestBounceValidation();

UFUNCTION(Exec)
void ResetBounceState();
```

## Performance Benchmarks

### Target Performance Metrics
- Attribute access: < 0.1ms per call
- Validation check: < 0.2ms per call  
- Full bounce execution: < 1ms total
- Memory usage: Negligible overhead vs cached variables

### Profiling Points
1. `GetCurrentAirBounceCount()` execution time
2. `ValidateActivationRequirements()` execution time
3. Network replication bandwidth impact
4. Memory allocation patterns

## Debugging Tools

### Console Commands
- `showdebug abilitysystem` - Show all attribute values
- `showdebug gas` - General GAS debug info
- `AbilitySystem.DebugAttributeSet MyAttributeSet` - Specific attribute set debug

### Log Categories
```cpp
DECLARE_LOG_CATEGORY_EXTERN(LogBounceAbility, Log, All);
// Use: UE_LOG(LogBounceAbility, Log, TEXT("Debug message"));
```

### Debug Visualization
Enable bounce debug drawing:
```cpp
bEnableBounceDebugDraw = true;
DebugDrawDuration = 5.0f;
```

## Quality Assurance Checklist

### Code Quality
- [ ] All functions have appropriate const modifiers
- [ ] Proper error handling for edge cases
- [ ] Consistent naming conventions (Epic Games style)
- [ ] Comprehensive logging for debugging
- [ ] Blueprint integration tested
- [ ] Network replication verified

### Functionality 
- [ ] Air bounce counting works correctly
- [ ] Ground contact resets attribute properly
- [ ] Validation logic matches execution logic
- [ ] Physics integration maintains expected behavior
- [ ] No regressions from previous implementation

### Performance
- [ ] No significant performance degradation
- [ ] Memory usage remains acceptable
- [ ] Network traffic impact is minimal
- [ ] Attribute access is efficient

## Known Issues and Limitations

### Current Limitations
1. Attribute values are stored as float but used as int (design choice for GAS compatibility)
2. Static delegate setup may not work correctly in multiplayer scenarios
3. Debug visualization still uses cached variables in some places

### Future Improvements
1. Implement proper multiplayer attribute replication testing
2. Add comprehensive automated test suite
3. Create Blueprint testing framework
4. Add performance profiling automation

## Conclusion

This testing framework ensures the bounce system upgrade to Gameplay Attributes maintains reliability, performance, and Epic Games coding standards while providing better maintainability and testability for future development.
