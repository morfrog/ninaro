// The Modern look's window and control drawing. Each helper draws the Modern
// version when the Display option is Modern, and otherwise the VGA version
// the original used (the palette slots passed in), so VGA and Monochrome are
// unchanged.

import { BORDER, FILL, type Graph } from './graph';
import { borderedBox } from './screen';

/** The Modern colours for controls and windows (the game's own colours come from the palette). */
export const MODERN = {
  face: '#2d2d30',
  raisedLight: '#4a4a4f',
  raisedDark: '#0e0e10',
  field: '#1e1e1e',
  text: '#f0f0f0',
  dim: '#7a7a80',
  accent: '#0078d4',
  accentText: '#ffffff',
  shadow: 'rgba(0,0,0,0.45)',
} as const;

export function isModern(g: Graph): boolean {
  return g.displayMode === 'modern';
}

/** The text colour for dialog and menu text: Modern's, or the slot the original used. */
export function inkText(g: Graph, slot: number): void {
  if (isModern(g)) g.setColorCss(MODERN.text);
  else g.setColor(slot);
}

/** The colour of a selected item: Modern's accent, or the Game Options' hilite. */
export function inkSelected(g: Graph, hilite: number): void {
  if (isModern(g)) g.setColorCss(MODERN.accent);
  else g.setColor(hilite);
}

/** A filled panel (a dialog's or menu's background). */
export function panelFill(g: Graph, x1: number, y1: number, x2: number, y2: number, slot: number): void {
  if (isModern(g)) g.setColorCss(MODERN.face);
  else g.setColor(slot);
  g.rectangle(FILL, x1, y1, x2, y2);
}

/** A thin outline, drawn in the current colour. */
function edge(g: Graph, x1: number, y1: number, x2: number, y2: number): void {
  g.rectangle(FILL, x1, y1, x2, y1); // top
  g.rectangle(FILL, x1, y2, x2, y2); // bottom
  g.rectangle(FILL, x1, y1, x1, y2); // left
  g.rectangle(FILL, x2, y1, x2, y2); // right
}

/**
 * A raised 3D control (a button, a slider's thumb): lit at the top and left,
 * shaded at the bottom and right. VGA: a bordered box in the two slots.
 */
export function raised(g: Graph, x1: number, y1: number, x2: number, y2: number, face: number, hilite: number): void {
  if (!isModern(g)) {
    borderedBox(g, x1, y1, x2, y2, face, hilite);
    return;
  }
  g.setColorCss(MODERN.face);
  g.rectangle(FILL, x1, y1, x2, y2);
  g.setColorCss(MODERN.raisedLight);
  g.rectangle(FILL, x1, y1, x2 - 1, y1);
  g.rectangle(FILL, x1, y1, x1, y2 - 1);
  g.setColorCss(MODERN.raisedDark);
  g.rectangle(FILL, x1 + 1, y2, x2, y2);
  g.rectangle(FILL, x2, y1 + 1, x2, y2);
}

/**
 * A window or menu box with a border. VGA: the bordered box; Modern: a soft
 * shadow, the face, and a thin dark edge.
 */
export function windowBox(g: Graph, x1: number, y1: number, x2: number, y2: number, face: number, edgeSlot: number): void {
  if (!isModern(g)) {
    borderedBox(g, x1, y1, x2, y2, face, edgeSlot);
    return;
  }
  g.setColorCss(MODERN.shadow);
  g.rectangle(FILL, x1 + 4, y1 + 4, x2 + 4, y2 + 4);
  g.setColorCss(MODERN.face);
  g.rectangle(FILL, x1, y1, x2, y2);
  g.setColorCss(MODERN.raisedDark);
  edge(g, x1, y1, x2, y2);
}

/** Marks the selected control's frame: VGA's hilite lines, Modern's accent outline. */
export function selectionFrame(g: Graph, x1: number, y1: number, x2: number, y2: number, hilite: number): void {
  if (isModern(g)) {
    g.setColorCss(MODERN.accent);
    edge(g, x1, y1, x2, y2);
    return;
  }
  g.setColor(hilite);
  g.rectangle(BORDER, x1, y1, x2, y2);
  g.rectangle(BORDER, x1 + 1, y1 - 1, x2 - 1, y2 + 1);
}

/** A radio button: a ring with a dot in it when selected (Modern), else the VGA ring and fill. */
export function radioMark(g: Graph, x1: number, y1: number, x2: number, y2: number, selected: boolean, slot: number): void {
  if (!isModern(g)) {
    g.setColor(slot);
    g.ellipse(BORDER, x1, y1, x2, y2);
    if (selected) {
      const x = Math.trunc((x2 - x1) / 4);
      const y = Math.trunc((y2 - y1) / 4);
      g.ellipse(FILL, x1 + x, y1 + y, x2 - x, y2 - y);
    }
    return;
  }
  // A true circle, centred in the box: the box's shorter side is its diameter.
  const d = Math.min(x2 - x1, y2 - y1);
  const cx = Math.trunc((x1 + x2) / 2);
  const cy = Math.trunc((y1 + y2) / 2);
  const r = Math.trunc(d / 2);
  g.setColorCss(MODERN.field);
  g.ellipse(FILL, cx - r, cy - r, cx + r, cy + r);
  g.setColorCss(MODERN.raisedDark);
  g.ellipse(BORDER, cx - r, cy - r, cx + r, cy + r);
  if (selected) {
    const inner = Math.trunc(r / 2);
    g.setColorCss(MODERN.accent);
    g.ellipse(FILL, cx - inner, cy - inner, cx + inner, cy + inner);
  }
}

/** A slider's track line, or a tick: Modern's dim line, VGA's text colour. */
export function trackLine(g: Graph, slot: number): void {
  if (isModern(g)) g.setColorCss(MODERN.dim);
  else g.setColor(slot);
}
