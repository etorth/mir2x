"""Right info pane of followuidmagiceditor, one small widget per section."""

from PySide6.QtCore import Qt, Signal
from PySide6.QtWidgets import (QAbstractItemView, QFormLayout, QGroupBox, QHeaderView, QLabel, QTableWidget,
                               QTableWidgetItem, QVBoxLayout, QWidget)

NONE_TEXT = "-"


def point_text(point):
    return NONE_TEXT if point is None else f"({point[0]}, {point[1]})"


class FormSection(QGroupBox):
    """A group box of "label: value" rows, values are selectable and wrapped."""

    FIELDS = ()

    def __init__(self, title, parent=None):
        super().__init__(title, parent)
        layout = QFormLayout(self)
        layout.setFieldGrowthPolicy(QFormLayout.FieldGrowthPolicy.AllNonFixedFieldsGrow)

        self.values = {}
        for field in self.FIELDS:
            value = QLabel(NONE_TEXT)
            value.setWordWrap(True)
            value.setTextInteractionFlags(Qt.TextInteractionFlag.TextSelectableByMouse)
            layout.addRow(f"{field}:", value)
            self.values[field] = value

    def set_values(self, **values):
        for field, label in self.values.items():
            label.setText(str(values.get(field.lower().replace(" ", "_").replace("-", "_"), NONE_TEXT)))


class MagicDirSection(FormSection):
    FIELDS = ("Path", "APNGs", "Unsaved")

    def __init__(self, parent=None):
        super().__init__("Magic Dir", parent)

    def update_dir(self, magic_dir):
        if magic_dir is None:
            self.set_values(path="none, use File > Open Magic Dir")
        else:
            self.set_values(path=magic_dir.path, apngs=len(magic_dir.seqs), unsaved=len(magic_dir.dirty_seqs()))


class MagicSection(FormSection):
    FIELDS = ("Name", "ID", "Element", "Ref", "Shared with")

    def __init__(self, parent=None):
        super().__init__("Magic", parent)

    def update_magic(self, name, record, entry, shared):
        if name is None:
            self.set_values()
            return

        ref = entry["ref"]
        self.set_values(name=name, id=record["id"], element=record["elem"] or NONE_TEXT,
                        ref=f"{ref['name']} {ref['stage']}" if ref else NONE_TEXT,
                        shared_with=", ".join(shared) if shared else NONE_TEXT)


class GfxEntrySection(FormSection):
    FIELDS = ("gfxID", "Stage", "Type", "Directions", "Frames", "Speed", "Loop", "Motion", "On ground", "Mod color",
              "Non-rotate", "Missing")

    def __init__(self, parent=None):
        super().__init__("Gfx Entry", parent)

    def update_entry(self, entry, frame_count, non_rotate, missing):
        if entry is None:
            self.set_values()
            return

        frames = entry["frameCount"] if entry["frameCount"] else f"0, all {frame_count} of APNG"
        self.set_values(gfxid=f"{entry['gfxID']:07X}", stage=entry["stage"], type=entry["type"],
                        directions=entry["gfxDirType"], frames=frames, speed=entry["speed"], loop=entry["loop"],
                        motion=entry["motion"], on_ground=entry["onGround"], mod_color=f"0x{entry['modColor']:08X}",
                        non_rotate="yes" if non_rotate else "no",
                        missing=", ".join(map(str, missing)) if missing else NONE_TEXT)


class HoverSection(FormSection):
    FIELDS = ("Direction", "Mouse", "Head", "Tail", "Saved head", "Saved tail", "File", "Edited name")

    def __init__(self, parent=None):
        super().__init__("Hovered Direction", parent)

    def update_hover(self, seq, direction, mouse):
        if seq is None:
            self.set_values()
            return

        self.set_values(direction=direction, mouse=point_text(mouse), head=point_text(seq.head),
                        tail=point_text(seq.tail), saved_head=point_text(seq.saved_head),
                        saved_tail=point_text(seq.saved_tail), file=seq.name,
                        edited_name=seq.edited_name() if seq.dirty else NONE_TEXT)


class DirectionTable(QGroupBox):
    """Directions of the current gfx, clicking a row selects the direction."""

    COLUMNS = ("Dir", "dx", "dy", "Head", "Tail", "Frames", "Size", "*")
    directionSelected = Signal(object)

    def __init__(self, parent=None):
        super().__init__("Directions", parent)
        self.table = QTableWidget(0, len(self.COLUMNS))
        self.table.setHorizontalHeaderLabels(self.COLUMNS)
        self.table.horizontalHeaderItem(len(self.COLUMNS) - 1).setToolTip("unsaved")
        self.table.verticalHeader().hide()
        self.table.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        self.table.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectRows)
        self.table.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        self.table.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.ResizeToContents)
        self.table.itemSelectionChanged.connect(self._on_selection_changed)

        layout = QVBoxLayout(self)
        layout.addWidget(self.table)

    def update_seqs(self, seqs):
        """seqs[d] is the MagicSeq of direction d, or None if it has no APNG."""
        selected = self.selected_direction()
        self.table.blockSignals(True)
        self.table.setRowCount(len(seqs))

        for d, seq in enumerate(seqs):
            if seq is None:
                cells = (d, "missing", "", "", "", "", "", "")
            else:
                frames = seq.frames()
                size = f"{frames[0].width()}x{frames[0].height()}" if frames else NONE_TEXT
                cells = (d, seq.dx, seq.dy, point_text(seq.head), point_text(seq.tail), len(frames), size,
                         "*" if seq.dirty else "")

            for column, text in enumerate(cells):
                item = QTableWidgetItem(str(text))
                item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
                self.table.setItem(d, column, item)

        if selected is not None and selected < len(seqs):
            self.table.selectRow(selected)
        else:
            self.table.clearSelection()

        self.table.blockSignals(False)
        self._fit_height()

    def clear_selection(self):
        self.table.clearSelection()

    def selected_direction(self):
        rows = self.table.selectionModel().selectedRows()
        return rows[0].row() if rows else None

    def _on_selection_changed(self):
        self.directionSelected.emit(self.selected_direction())

    def _fit_height(self):
        height = self.table.horizontalHeader().height() + 2 * self.table.frameWidth()
        height += sum(self.table.rowHeight(r) for r in range(self.table.rowCount()))
        if self.table.horizontalScrollBar().isVisible() or \
                self.table.horizontalHeader().length() > self.table.viewport().width():
            height += self.table.horizontalScrollBar().sizeHint().height()
        self.table.setFixedHeight(height)


class InfoPane(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.dir_section = MagicDirSection()
        self.magic_section = MagicSection()
        self.entry_section = GfxEntrySection()
        self.hover_section = HoverSection()
        self.direction_table = DirectionTable()

        layout = QVBoxLayout(self)
        for section in (self.dir_section, self.magic_section, self.entry_section, self.hover_section,
                        self.direction_table):
            layout.addWidget(section)
        layout.addStretch(1)
