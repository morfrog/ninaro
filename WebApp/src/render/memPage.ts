// MEMPAGE.C: a saved copy of the screen (the original used _getimage and
// _putimage), so menus and dialogs can be drawn over it and taken away.

export class MemPage {
  private copy: HTMLCanvasElement | null = null;

  constructor(private readonly canvas: HTMLCanvasElement) {}

  /** MemPageGet: remember the screen as it is. */
  get(): void {
    const copy = this.copy ?? document.createElement('canvas');
    copy.width = this.canvas.width;
    copy.height = this.canvas.height;
    copy.getContext('2d')!.drawImage(this.canvas, 0, 0);
    this.copy = copy;
  }

  /** MemPagePut: put the remembered screen back. */
  put(): void {
    if (!this.copy || this.copy.width !== this.canvas.width || this.copy.height !== this.canvas.height) return;
    const ctx = this.canvas.getContext('2d')!;
    ctx.save();
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.drawImage(this.copy, 0, 0);
    ctx.restore();
  }

  /** Whether put() can restore the screen (false after a resize). */
  get valid(): boolean {
    return this.copy !== null && this.copy.width === this.canvas.width && this.copy.height === this.canvas.height;
  }
}
