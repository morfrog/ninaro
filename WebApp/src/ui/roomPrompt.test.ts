import { describe, expect, it } from 'vitest';
import helpText from '../../../Legacy/NINARO.HLP?raw';
import { withOnlineHelp } from '../game/onlineHelp';
import { initialRoom, roomFromUrl, roomLink } from './roomPrompt';

describe('the room', () => {
  it('comes from ?online=, cleaned', () => {
    expect(roomFromUrl('?online=Smithy')).toBe('smithy');
    expect(roomFromUrl('?online=Smithy%20%26%20Co')).toBe('smithy-co');
    expect(roomFromUrl('?online=')).toBe('');
    expect(roomFromUrl('?online')).toBe('');
    expect(roomFromUrl('?other=1')).toBeNull();
    expect(roomFromUrl('')).toBeNull();
  });

  it('is offered from the URL first, then the default', () => {
    expect(initialRoom('?online=smithy')).toBe('smithy');
    expect(initialRoom('')).toBe('ninaro'); // no storage in the tests
  });

  it('makes a link to this page with the room in it', () => {
    expect(roomLink('Smithy', 'https://morfrog.github.io/ninaro/?online=old#x')).toBe('https://morfrog.github.io/ninaro/?online=smithy');
    expect(roomLink('', 'http://localhost:5174/')).toBe('http://localhost:5174/?online=ninaro');
  });
});

describe('the help', () => {
  it('gains Playing Online before Troubleshooting, in lines that fit the help box', () => {
    const text = withOnlineHelp(helpText);
    const online = text.indexOf('~Playing Online');
    expect(online).toBeGreaterThan(0);
    expect(online).toBeLessThan(text.indexOf('~Troubleshooting'));
    const section = text.slice(online, text.indexOf('~Troubleshooting'));
    for (const line of section.split('\n')) expect(line.length, line).toBeLessThanOrEqual(60);
  });
});
