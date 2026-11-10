// Players and player groups (Legacy PLAYER.C).

/** Legacy player type enum {Computer, Human}, and Online: an opponent on another device. */
export const PlayerType = { Computer: 0, Human: 1, Online: 2 } as const;
export type PlayerType = (typeof PlayerType)[keyof typeof PlayerType];

export interface Player {
  /** Up to 19 characters in the original (char name[20]). */
  name: string;
  /** Palette index 0..15. */
  color: number;
  type: PlayerType;
  /** Index in its player group (PlayerGroupNew sets it). */
  pid: number;
}

/** PlayerNew */
export function playerNew(name: string, color: number, type: PlayerType): Player {
  return { name, color, type, pid: -1 };
}

/** PlayerCopy: name, colour and type; the move function goes with the type. */
export function playerCopy(dest: Player, source: Player): void {
  dest.name = source.name;
  dest.color = source.color;
  dest.type = source.type;
}

export class PlayerGroup {
  readonly players: Player[];

  /** PlayerGroupNew: each player's pid becomes its index. */
  constructor(players: Player[]) {
    this.players = players;
    players.forEach((p, i) => (p.pid = i));
  }

  /** PlayerGroupPlayer: null if index is out of range (the C only checked the end). */
  player(index: number): Player | null {
    return index < 0 || index >= this.players.length ? null : this.players[index];
  }

  /** PlayerGroupFind: the player's pid (the original stopped searching). */
  find(player: Player): number {
    return player.pid;
  }

  /** PlayerGroupWho: index of the first player with this name, or -1. */
  who(name: string): number {
    return this.players.findIndex((p) => p.name === name);
  }
}
