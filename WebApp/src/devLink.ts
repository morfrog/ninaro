// Classic mode exists only under the dev server (see dev/classicPlugin.ts).
if (import.meta.env.DEV) {
  const footer = document.querySelector('.site-footer');
  const link = document.createElement('a');
  link.href = 'classic.html';
  link.textContent = 'Classic (dev only)';
  footer?.append(' · ', link);
}

export {};
