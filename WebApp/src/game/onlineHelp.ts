// The help's section on playing online, which the original didn't have. It
// goes in before Troubleshooting; NINARO.HLP itself stays as the original.

const ONLINE_HELP = `~Playing Online
Two people can play each other on two devices (computers,
phones or tablets), each with its own screen.

In The Player Dialog choose Online for player one or two.
A box asks for a room name, such as smithy. The other player
does the same, with the same room name, on the same size
board. Copy link copies a link to the game with the room in
it, to send to the other player: opening it goes straight
to that room.

The first to arrive waits for the other. When both are
there, the boards are checked: if their size or the number
to connect differ, both are told and nothing starts. If they
match, both boards are cleared and the game starts, with the
first player chosen at random. After that, players take
turns to go first.

Each player keeps their own colour, on both screens. If both
chose the same colour, the second to arrive gets another.

The other player on your device can be a human or any of
the AIs, to let an AI play someone online.

Changing a player, the board or your colour, ending the game
or closing the page leaves the online game, and the other
player is told. Saving and loading games are off while
online. If the connection is lost, the game keeps trying:
press Esc for the menu, or F4 to end the game.

Anyone who knows the room name can join it, so pick one
that is hard to guess. Two others already playing in a room
keep it until they leave.

`;

/** The help text with Playing Online added before Troubleshooting. */
export function withOnlineHelp(helpText: string): string {
  const at = helpText.indexOf('~Troubleshooting');
  if (at < 0) return helpText + '\n' + ONLINE_HELP;
  return helpText.slice(0, at) + ONLINE_HELP + helpText.slice(at);
}
