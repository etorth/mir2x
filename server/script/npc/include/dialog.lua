local dialogue = {}

-- Presentation primitives only: callers own their menus, event handlers and operations.
function dialogue.link(id, label, opts)
    opts = opts or {}

    local is_close = opts.close
    if is_close == nil then
        is_close = (id == SYS_EXIT)
    end

    local close_attr = is_close and ' close="1"' or ''
    local prefix = opts.prefix or ''
    local suffix = opts.suffix or ''

    return string.format('%s<event id="%s"%s>%s</event>%s', prefix, id, close_attr, label, suffix)
end

function dialogue.post(uid, text, choices)
    if type(text) == 'function' then
        text = text(uid)
    end
    assertType(text, 'table')

    local xml = {'<layout>'}
    for _, line in ipairs(text) do
        table.insert(xml, string.format('<par>%s</par>', line))
    end
    if choices then
        table.insert(xml, '<par></par>')
        for _, line in ipairs(choices) do
            table.insert(xml, string.format('<par>%s</par>', line))
        end
    end
    table.insert(xml, '</layout>')
    uidPostXML(uid, '%s', table.concat(xml, '\n'))
end

function dialogue.guardRedName(callback, text, exitLabel)
    if text == nil then
        return callback
    end
    return function(uid, value)
        if uidQueryRedName(uid) then
            dialogue.post(uid, type(text) == 'table' and text or {text},
                {dialogue.link(SYS_EXIT, exitLabel)})
        else
            return callback(uid, value)
        end
    end
end

return dialogue
