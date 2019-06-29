TestScriptProcess = class(ScriptProcess,
{
    count = 0;
});

function TestScriptProcess:OnInit()
    print("OnInit()");
end

function TestScriptProcess:OnUpdate(deltaMs)
    self.count = self.count + 1;
    print("Count: " .. self.count);

    if self.count >= t then
        self:Succeed();
    end
end

function TestScriptProcess:OnSuccess()
    print("Success!!");
end

-- run some tests
parent = TestScriptProcess:Create({frequency = 1000});
child = TestScriptProcess:Create({frequency = 500});
parent:AttachChild(child);

AttachProcess(parent);