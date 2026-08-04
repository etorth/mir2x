---
name: widget-class-design-and-refactor
description: Design or refactor mir2x GUI Widget classes using composition, small focused child widgets, and minimal drawDefault/processEventDefault overrides. Use for boards, dialogs, lists, rows, controls, or any GUI change involving Widget subclasses.
---

# Widget class design and refactoring

Use composition as the default design strategy for mir2x GUI code.

## Core principle

Build a large widget from many small widget objects. Prefer standard widget
composition over manually reconstructing an interface in a large
`drawDefault()` or `processEventDefault()` implementation.

A well-composed parent widget should normally:

- Own its child widgets as members.
- Configure their layout, visibility, state, callbacks, and dynamic values.
- Let `Widget::drawDefault()` draw the hierarchy.
- Let `Widget::processEventDefault()` route events through the hierarchy.
- Contain application-level coordination, not low-level rendering or hit
  testing for every visual element.

## Decomposition rules

Split a GUI into the smallest useful pieces:

- A board is composed of panels, lists, buttons, labels, and detail widgets.
- A list is composed of an `ItemBox` and small row widgets.
- A row is composed of individual `TextBoard`, `ImageBoard`, button, and
  background widgets.
- Repeated visual or interactive behavior belongs in a reusable child class,
  not in a loop inside the parent board's drawing or event function.
- State should live in the smallest widget that owns the behavior.
- Expose only the state the parent needs, such as selected or hovered indexes.

Prefer existing composition helpers before adding custom drawing or event code:

- `ItemBox`, `ItemPair`, and `MarginContainer`
- `ScrollContainer`
- `LayoutBoard` and `TextBoard`
- `ImageBoard` and `GfxShapeBoard`
- Existing button classes
- `Widget::Var*` values and lambdas for dynamic position, size, text, color,
  visibility, and active state

## Override policy

Do not override `drawDefault()` or `processEventDefault()` merely because it is
possible.

Override them only when normal composition cannot express the required
behavior. When an override is necessary:

1. Put it in the smallest leaf or near-leaf widget that owns the behavior.
2. Keep the implementation narrow and easy to understand.
3. Continue using child widgets for all ordinary drawing and interaction.
4. Delegate unhandled behavior to the base implementation.

Examples:

- A row may override `processEventDefault()` to track hover and selection.
- A list may override `processEventDefault()` to translate the mouse wheel into
  a change of `firstIndex`.
- A tiny specialized canvas may override `drawDefault()` for rendering that
  cannot be represented by `ImageBoard`, `TextBoard`, or `GfxShapeBoard`.
- The containing board should still compose these pieces instead of duplicating
  their logic.

## Event-handling rules

- Call `m.calibrate(this)` before using the supplied `ROIMap`.
- Never consume an invalid event.
- Let interactive children receive events before implementing parent-level
  fallback behavior.
- Consume an event only when the widget actually owns that interaction.
- Hover tracking generally should not consume mouse motion if the parent still
  needs the motion, such as for dragging the containing board.
- Preserve pointer-capture semantics for an interaction from press through
  release.
- Call `Widget::processEventDefault()` for events not handled by the specialized
  logic.
- Avoid repeating coordinate conversion and manual hit-testing in a large
  parent when child widget geometry can perform the routing.

## Drawing rules

- Prefer `TextBoard` for text, `ImageBoard` for textures, and `GfxShapeBoard`
  for simple backgrounds, borders, and highlights.
- Use widget direction and position to align content instead of manually
  calculating draw destinations every frame.
- Use `Widget::Var*` lambdas when content or appearance depends on shared state.
- If `drawDefault()` must be overridden, preserve composed children by calling
  `Widget::drawDefault()` at the correct layer.
- Keep drawing order explicit through child construction/order rather than one
  large manual drawing procedure.
- Derive fixed-frame widget dimensions from the frame texture when possible.

## Lists and large result sets

Do not create one widget for every result when a result set can be large.

Use a virtualized fixed-size row pool:

1. Keep the complete data list in the list widget.
2. Keep a `firstIndex`.
3. Create only the number of rows that can be visible.
4. Give each row a fixed `rowIndex`.
5. Resolve row data as `data[firstIndex + rowIndex]`.
6. Render an empty row when that index is out of range.
7. Change `firstIndex` when scrolling instead of rebuilding the widget tree.
8. Return absolute selected and hovered data indexes to the parent.

This keeps widget count and event traversal bounded regardless of query size.

## Refactoring workflow

When refactoring an existing manually implemented widget:

1. Identify independent visual and interactive regions.
2. Search for existing widget classes that already model each region.
3. Extract repeated rows, cells, panels, or controls into small classes.
4. Move local state and callbacks into the extracted class.
5. Replace manual drawing with composed child widgets.
6. Replace parent hit-testing with child event handling.
7. Keep only unavoidable coordination in the parent.
8. Preserve existing geometry, behavior, focus handling, paging, and selection.
9. Validate empty, partial, full, oversized, selected, hovered, and disabled
   states.

## Avoid these designs

- A board that manually draws every label, image, row, and highlight.
- A board that manually hit-tests every child-shaped region.
- A single large widget class responsible for unrelated GUI regions.
- Rebuilding a large widget tree whenever the displayed data offset changes.
- Duplicating behavior already available through an existing widget.
- An override that prevents composed children from drawing or receiving events.
- Event handlers that consume mouse motion unnecessarily and break parent
  dragging or sibling interaction.

The goal is not to eliminate every override. The goal is to keep unavoidable
custom behavior small, local, composable, and easy to verify.
