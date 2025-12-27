import React from 'react';

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
    onPlace?: (r: number, c: number) => void;
}

// Board renders a square board with visible grid lines.
// Player is positioned on intersections (0..size) for both axes.
export default function Board({ size = 8, myId = null, members = [], board = [], onPlace }: BoardProps) {
    // size = number of cells per side; intersections = size + 1
    const [hoverPos, setHoverPos] = React.useState<{ r: number, c: number } | null>(null);

    function toGridCoords(e: React.MouseEvent<HTMLDivElement, MouseEvent>) {
        const el = e.currentTarget as HTMLDivElement;
        const rect = el.getBoundingClientRect();
        const relX = e.clientX - rect.left;
        const relY = e.clientY - rect.top;
        const leftRatio = relX / rect.width;
        const topRatio = relY / rect.height;
        let r = Math.round(topRatio * size);
        let c = Math.round(leftRatio * size);
        if (r < 0) r = 0;
        if (c < 0) c = 0;
        if (r > size - 1) r = size - 1;
        if (c > size - 1) c = size - 1;
        return { r, c };
    }

    return (
        <div
            className="board board-lines"
            style={{ ['--size' as any]: size }}
            onMouseMove={(e) => {
                const pos = toGridCoords(e);
                setHoverPos(pos);
            }}
            onMouseLeave={() => setHoverPos(null)}
            onClick={(e) => {
                const pos = toGridCoords(e);
                if (onPlace) onPlace(pos.r, pos.c);
            }}
        >
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
            {/* Mouse hover preview for my cursor */}
            {hoverPos && (() => {
                const idx = members ? members.indexOf(myId || '') : -1;
                const myColor = idx === 0 ? 'black' : idx === 1 ? 'white' : null;
                if (!myColor) return null;
                const topPct = (hoverPos.r / size) * 100;
                const leftPct = (hoverPos.c / size) * 100;
                return (
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
                            opacity: 0.6,
                            transform: 'translate(-50%, -50%)',
                            zIndex: 5,
                            border: '1px dashed #333'
                        }}
                    />
                )
            })()}
        </div>
    )
}