local dialog = {}

function dialog.link(id, label, opts)
    assertType(id, 'string')
    assertType(label, 'string')
    assertType(opts, 'table', 'nil')

    opts = opts or {}

    local is_close = opts.close
    if is_close == nil then
        is_close = (id == SYS_EXIT)
    end

    local args_attr  = opts.args  ~= nil and string.format(' args="%s"', opts.args)  or ''
    local wrap_attr  = opts.wrap  ~= nil and string.format(' wrap="%s"', tostring(opts.wrap)) or ''
    local close_attr = is_close and ' close="1"' or ''
    local prefix = opts.prefix or ''
    local suffix = opts.suffix or ''

    return string.format('%s<event id="%s"%s%s%s>%s</event>%s', prefix, id, args_attr, wrap_attr, close_attr, label, suffix)
end

local function renderPar(line)
    if type(line) == 'table' then
        local align_attr = line.align ~= nil and string.format(' align="%s"', line.align) or ''
        return string.format('<par%s>%s</par>', align_attr, assertType(line[1], 'string'))
    end
    return string.format('<par>%s</par>', assertType(line, 'string'))
end

function dialog.post(uid, arg1, arg2, arg3)
    assertType(uid, 'integer')

    local eventPath
    local text
    local choices

    if isArray(arg1) and ((arg1[1] == SYS_EPUID) or (arg1[1] == SYS_EPQST) or (arg1[1] == SYS_EPDEF)) then
        eventPath = arg1
        text = arg2
        choices = arg3
    else
        eventPath = {SYS_EPDEF}
        text = arg1
        choices = arg2
        assertType(arg3, 'nil')
    end

    assertType(text, 'function', 'table', 'string')
    assertType(choices, 'function', 'table', 'string', 'nil')

    if type(text) == 'function' then
        text = assertType(text(uid), 'table', 'string')
    end

    if type(text) == 'string' then
        text = {text}
    end

    local xml = {'<layout>'}
    for _, line in ipairs(text) do
        table.insert(xml, renderPar(line))
    end

    if choices then
        if type(choices) == 'function' then
            choices = assertType(choices(uid), 'table', 'string')
        end

        if type(choices) == 'string' then
            choices = {choices}
        end

        table.insert(xml, '<par></par>')
        for _, line in ipairs(choices) do
            table.insert(xml, renderPar(line))
        end
    end

    table.insert(xml, '</layout>')
    uidPostXML(uid, eventPath, '%s', table.concat(xml, '\n'))
end

function dialog.guardRedName(callback, text, exitLabel)
    assertType(callback, 'function')
    if text == nil then
        return callback
    end

    assertType(text, 'function', 'table', 'string')
    assertType(exitLabel, 'function', 'string', 'nil')

    return function(uid, value)
        if uidQueryRedName(uid) then
            if type(exitLabel) == 'function' then
                exitLabel = assertType(exitLabel(uid), 'string')
            elseif exitLabel == nil then
                exitLabel = '关闭'
            end
            dialog.post(uid, text,
            dialog.link(SYS_EXIT, exitLabel))
        else
            return callback(uid, value)
        end
    end
end

return dialog
