"""Canvas of followuidmagiceditor: all directions of one magic gfx on a circle.

Direction i of N is placed at i * 360 / N degrees clockwise from up, its frame is drawn at anchor + (dx, dy).
The anchors are on the gray circle, a yellow "*" label is an unsaved direction.
The direction under the mouse, which a click edits, has a green border, the selected one is orange.
Head (red) and tail (cyan) are anchor relative, the tail -> head line is yellow.

    left click/drag  : set head of the direction under mouse
    right click/drag : set tail of the direction under mouse
    drag elsewhere   : move the view
    ctrl + drag      : change the circle radius
    wheel            : zoom around the mouse

A drag keeps editing the direction it starts on, the point is clamped to that frame.
"""

import math

from PySide6.QtCore import QPointF, QRectF, Qt, Signal
from PySide6.QtGui import QColor, QPainter, QPen
from PySide6.QtWidgets import QWidget

DEFAULT_RADIUS = 160.0
DEFAULT_ZOOM = 1.0
MIN_ZOOM = 0.25
MAX_ZOOM = 8.0


class MagicCanvas(QWidget):
    # direction, "head" or "tail", (x, y) relative to the anchor
    pointPicked = Signal(int, str, tuple)
    # None, or (direction, (x, y) of the mouse relative to its anchor)
    hoverChanged = Signal(object)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMouseTracking(True)
        self.setMinimumSize(480, 480)

        self.magic_dir = None
        self.gfx_id = None
        self.dir_count = 0
        self.frame = 0
        self.radius = DEFAULT_RADIUS
        self.zoom = DEFAULT_ZOOM
        self._dragging_radius = False
        self._drag = None  # (button, direction, "head" or "tail") while dragging a point
        self._pan_drag = None  # (button, mouse start, pan start) while moving the view
        self.pan = QPointF(0, 0)
        self._mouse_pos = None
        self.hover_direction = None
        self.selected_direction = None

    def set_magic(self, magic_dir, gfx_id, dir_count):
        self.magic_dir = magic_dir
        self.gfx_id = gfx_id
        self.dir_count = dir_count
        self._drag = None
        self.selected_direction = None
        self._update_hover()
        self.update()

    def set_selected(self, direction):
        self.selected_direction = direction
        self.update()

    def set_frame(self, frame):
        self.frame = frame
        self.update()

    def reset_view(self):
        self.radius = DEFAULT_RADIUS
        self.zoom = DEFAULT_ZOOM
        self.pan = QPointF(0, 0)
        self._update_hover()
        self.update()

    def anchor(self, direction):
        if self.dir_count <= 1:
            return QPointF(0, 0)
        angle = 2 * math.pi * direction / self.dir_count
        return QPointF(self.radius * math.sin(angle), -self.radius * math.cos(angle))

    def seq(self, direction):
        if self.magic_dir is None or self.gfx_id is None:
            return None
        return self.magic_dir.get(self.gfx_id, direction)

    def frame_rect(self, direction):
        seq = self.seq(direction)
        image = seq.frame(self.frame) if seq else None
        if image is None:
            return None
        return QRectF(self.anchor(direction) + QPointF(seq.dx, seq.dy), image.size().toSizeF())

    def origin(self):
        return QPointF(self.width() / 2, self.height() / 2) + self.pan

    def to_scene(self, pos):
        return (QPointF(pos) - self.origin()) / self.zoom

    def direction_at(self, scene_pos):
        best = None
        for direction in range(self.dir_count):
            rect = self.frame_rect(direction)
            if rect is None or not rect.contains(scene_pos):
                continue

            offset = scene_pos - self.anchor(direction)
            distance = offset.x() ** 2 + offset.y() ** 2
            if best is None or distance < best[0]:
                best = (distance, direction)
        return None if best is None else best[1]

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.fillRect(self.rect(), Qt.GlobalColor.black)
        painter.translate(self.origin())
        painter.scale(self.zoom, self.zoom)

        if self.dir_count > 1:
            pen = QPen(QColor(60, 60, 60))
            pen.setCosmetic(True)
            painter.setPen(pen)
            painter.drawEllipse(QPointF(0, 0), self.radius, self.radius)

        on_top = [d for d in (self.selected_direction, self.hover_direction) if d is not None]
        for direction in range(self.dir_count):
            if direction not in on_top:
                self._draw_direction(painter, direction)

        # on top, so their borders aren't covered by overlapping neighbours
        for direction in dict.fromkeys(on_top):
            self._draw_direction(painter, direction)

    def _draw_direction(self, painter, direction):
        anchor = self.anchor(direction)
        seq = self.seq(direction)

        pen = QPen()
        pen.setCosmetic(True)

        if seq is None or seq.frame(self.frame) is None:
            pen.setColor(QColor("red"))
            painter.setPen(pen)
            painter.drawLine(anchor + QPointF(-8, -8), anchor + QPointF(8, 8))
            painter.drawLine(anchor + QPointF(-8, 8), anchor + QPointF(8, -8))
            painter.drawText(anchor + QPointF(10, -10), str(direction))
            return

        rect = self.frame_rect(direction)
        painter.drawImage(rect.topLeft(), seq.frame(self.frame))

        if direction == self.hover_direction:
            pen.setColor(QColor("lime"))
        elif direction == self.selected_direction:
            pen.setColor(QColor("orange"))
        else:
            pen.setColor(QColor(80, 80, 80))
        painter.setPen(pen)
        painter.drawRect(rect)

        pen.setColor(QColor("yellow") if seq.dirty else QColor("white"))
        painter.setPen(pen)
        painter.drawText(rect.topLeft() + QPointF(2, -2), f"{direction}*" if seq.dirty else str(direction))
        self._draw_cross(painter, anchor, 3)

        head = anchor + QPointF(*seq.head)
        tail = anchor + QPointF(*seq.tail)

        pen.setColor(QColor("yellow"))
        painter.setPen(pen)
        painter.drawLine(tail, head)

        pen.setColor(QColor("cyan"))
        painter.setPen(pen)
        self._draw_cross(painter, tail, 5)

        pen.setColor(QColor("red"))
        painter.setPen(pen)
        self._draw_cross(painter, head, 5)

    @staticmethod
    def _draw_cross(painter, point, size):
        painter.drawLine(point + QPointF(-size, 0), point + QPointF(size, 0))
        painter.drawLine(point + QPointF(0, -size), point + QPointF(0, size))

    def mousePressEvent(self, event):
        if event.modifiers() & Qt.KeyboardModifier.ControlModifier and event.button() == Qt.MouseButton.LeftButton:
            self._dragging_radius = True
            self._update_radius(event.position())
            return

        which = {Qt.MouseButton.LeftButton: "head", Qt.MouseButton.RightButton: "tail"}.get(event.button())
        if which is None:
            return super().mousePressEvent(event)

        if self._drag is not None or self._pan_drag is not None:
            return

        direction = self.direction_at(self.to_scene(event.position()))
        if direction is None:
            self._pan_drag = (event.button(), event.position(), QPointF(self.pan))
            self.setCursor(Qt.CursorShape.ClosedHandCursor)
            return

        self._drag = (event.button(), direction, which)
        self._drag_point(event.position())

    def mouseMoveEvent(self, event):
        self._mouse_pos = event.position()
        if self._dragging_radius:
            self._update_radius(event.position())
        elif self._drag is not None:
            self._drag_point(event.position())
        elif self._pan_drag is not None:
            _, start, pan = self._pan_drag
            self.pan = pan + event.position() - start
            self.update()
        self._update_hover()

    def leaveEvent(self, event):
        self._mouse_pos = None
        self._update_hover()
        super().leaveEvent(event)

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self._dragging_radius = False
        if self._drag is not None and event.button() == self._drag[0]:
            self._drag = None
            self._update_hover()
        if self._pan_drag is not None and event.button() == self._pan_drag[0]:
            self._pan_drag = None
            self.unsetCursor()
        super().mouseReleaseEvent(event)

    def wheelEvent(self, event):
        factor = 1.25 if event.angleDelta().y() > 0 else 0.8
        pos = event.position()
        scene_pos = self.to_scene(pos)

        # keep the scene point under the mouse fixed
        self.zoom = min(MAX_ZOOM, max(MIN_ZOOM, self.zoom * factor))
        self.pan = pos - scene_pos * self.zoom - QPointF(self.width() / 2, self.height() / 2)
        self._update_hover()
        self.update()

    def _update_radius(self, pos):
        scene_pos = self.to_scene(pos)
        self.radius = math.hypot(scene_pos.x(), scene_pos.y())
        self.update()

    def _drag_point(self, pos):
        _, direction, which = self._drag
        rect = self.frame_rect(direction)
        if rect is None:
            return

        scene_pos = self.to_scene(pos)
        x = min(max(scene_pos.x(), rect.left()), rect.right() - 1)
        y = min(max(scene_pos.y(), rect.top()), rect.bottom() - 1)

        anchor = self.anchor(direction)
        point = (round(x - anchor.x()), round(y - anchor.y()))
        if point != getattr(self.seq(direction), which):
            self.pointPicked.emit(direction, which, point)

    def _update_hover(self):
        scene_pos = None if self._mouse_pos is None else self.to_scene(self._mouse_pos)
        if self._drag is not None:
            direction = self._drag[1]
        else:
            direction = None if scene_pos is None else self.direction_at(scene_pos)

        if direction != self.hover_direction:
            self.hover_direction = direction
            self.update()
        if direction is None or scene_pos is None:
            self.hoverChanged.emit(None)
        else:
            offset = scene_pos - self.anchor(direction)
            self.hoverChanged.emit((direction, (round(offset.x()), round(offset.y()))))
