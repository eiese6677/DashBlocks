import React from 'react'
import PropTypes from 'prop-types'

export default function Player({
    top,
    left,
    color,
    isMe,
    borderColor = '#333'
}) {
    return (
        <div
            className={`intersection-player ${isMe ? 'me' : ''}`}
            style={{
                top: top,
                left: left,
                background: color,
                border: isMe ? '2px solid gold' : `1px solid ${borderColor}`
            }}
        />
    )
}

Player.propTypes = {
    top: PropTypes.oneOfType([PropTypes.number, PropTypes.string]).isRequired,
    left: PropTypes.oneOfType([PropTypes.number, PropTypes.string]).isRequired,
    color: PropTypes.string,
    isMe: PropTypes.bool,
    borderColor: PropTypes.string
}
