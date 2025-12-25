import React from 'react';
import Player from './Player.jsx';

// Define types for the component props
interface PlayerData {
  [id: string]: [[number, number]];
}

interface Stone {
  r: number;
  c: number;
  color: 'black' | 'white';
}

interface BoardProps {
  size?: number;
  players?: PlayerData;
  myId?: string | null;
  members?: string[];
  board?: Stone[];
}

// Board renders a square board with visible grid lines.
// Player is positioned on intersections (0..size) for both axes.
export default function Board({ size = 8, players = {}, myId = null, members = [], board = [] }: BoardProps) {
    // size = number of cells per side; intersections = size + 1
    const intersections = size + 1;

    const palette = ['#ff4757', '#1e90ff', '#2ed573', '#ffa502', '#ff6b81', '#70a1ff', '#a55eea'];

    return (
        <div className="board board-lines" style={{ ['--size' as any]: size }}>
            {/* Render placed stones */}
            {board.map((item, i) => {
                const topPct = (item.r / size) * 100;
                const leftPct = (item.c / size) * 100;
                return (
                    <div
                        key={`stone-${i}`}
                        className="stone"
                        style={{
                            position: 'absolute',
                            top: `${topPct}%`,
                            left: `${leftPct}%`,
                            width: '24px',
                            height: '24px',
                            borderRadius: '50%',
                            backgroundColor: item.color === 'black' ? '#000' : '#fff',
                            border: '1px solid #777',
                            transform: 'translate(-50%, -50%)',
                            boxShadow: '1px 1px 3px rgba(0,0,0,0.5)'
                        }}
                    />
                )
            })}

            {/* Render Players (Cursor) and Preview */}
            {Object.entries(players).map(([id, data]) => {
                if (!data || !data[0]) return null;
                const [r, c] = data[0];
                const topPct = (r / size) * 100;
                const leftPct = (c / size) * 100;
                const isMe = id === myId;
                const idx = members ? members.indexOf(id) : -1;
                
                const myColor = idx === 0 ? 'black' : idx === 1 ? 'white' : null;

                return (
                    <React.Fragment key={id}>
                        {isMe && myColor && (
                            <div
                                className="stone-preview"
                                style={{
                                    position: 'absolute',
                                    top: `${topPct}%`,
                                    left: `${leftPct}%`,
                                    width: '24px',
                                    height: '24px',
                                    borderRadius: '50%',
                                    backgroundColor: myColor === 'black' ? '#000' : '#fff',
                                    opacity: 0.5,
                                    transform: 'translate(-50%, -50%)',
                                    zIndex: 1,
                                    border: '1px dashed #333'
                                }}
                            />
                        )}
                        <Player
                            top={`${topPct}%`}
                            left={`${leftPct}%`}
                            color={idx === 0 ? '#333' : idx === 1 ? '#ccc' : palette[id.length % palette.length]}
                            isMe={isMe}
                            borderColor={isMe ? '#ff0000' : '#000'} // Red border for cursor visibility
                        />
                    </React.Fragment>
                )
            })}
        </div>
    )
}