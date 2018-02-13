var selectN = require('bindings')('select');

module.exports = function select(readFds, writeFds = [], exceptFds = [], timeout = null) {
    const nfds = Math.max(...readFds, ...writeFds, ...exceptFds) + 1;
    return selectN(nfds, readFds, writeFds, exceptFds, timeout);
};