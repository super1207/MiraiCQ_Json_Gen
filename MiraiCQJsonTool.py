import tkinter as tk
from tkinter import ttk
import tkinter.messagebox
from tkinter import StringVar
from tkinter import Scrollbar
import tkinter.font as tf
import tkinter.filedialog
from ctypes import *
import time
import json
import os



def choose_event(window):
    f = tk.Toplevel()
    event_list = [
    "空事件",
    "框架启动,type=1001",
    "插件启用,type=1003",
    "私聊消息,type=21",
    "群消息,type=2",
    "群文件上传,type=11",
    "群管理员变动,type=101",
    "群成员减少,type=102",
    "群成员增加,type=103",
    "群禁言,type=104",
    "好友添加,type=201",
    "加好友请求,type=301",
    "加群请求／邀请,type=302",
    "插件停用,type=1004",
    "框架退出,type=1002",
    "菜单事件"]
    msf = ["空事件"]
    def deal(msg):
        msf[0] = msg
        f.destroy()
    for i in event_list:
        tk.Button(f, width=30, text=i, bg='Lavender', command=lambda x = i: deal(x)).pack()

    window.wait_window(f)
    def get_name(window):
        f = tk.Toplevel() 
        v1 = StringVar()
        tk.Label(f, text="菜单名:").pack()
        tk.Entry(f, textvariable =v1).pack()
        tk.Button(f,text="确定",command = lambda:f.destroy()).pack()
        window.wait_window(f)
        return v1.get()

    if msf[0] == '菜单事件':
        name = get_name(window)
        if name != '':
            msf[0] = '菜单事件'+",name=" + name
        else:
            msf[0] = "空事件"
    return msf[0]


window = tk.Tk()
window.title('Json Gen Tool(V1.0)')
window['background'] = 'PowderBlue'
window.resizable(0, 0)

inputText = tk.Entry(window, width=50,font=tf.Font(size=15) ,state='readonly')
inputText.grid(row=0, column=0, padx=8, pady=4)

columns = ("函数名(双击函数名可设置映射)", "函数含义")
treeview = ttk.Treeview(window, height=18, show="headings", columns=columns)
treeview.column(columns[0], width=255, anchor='center') 
treeview.column(columns[1], width=315, anchor='center')
treeview.heading(columns[0], text=columns[0]) # 显示表头
treeview.heading(columns[1], text=columns[1])
treeview.grid(row=1, column=0, columnspan=2, padx=8, pady=4)

VScroll1 = Scrollbar(treeview, orient='vertical', command=treeview.yview)
VScroll1.place(relx=0.971, rely=0.028, relwidth=0.024, relheight=0.958)
treeview.configure(yscrollcommand=VScroll1.set)

def save_json():
    x=treeview.get_children()
    event_fun = []
    menu_fun = []
    tid = 1
    for item in x:
        fun = treeview.item(item)['values'][0]
        value = treeview.item(item)['values'][1]
        if value == '空事件':
            continue
        if value.startswith("菜单事件,name="):
            name = value.split("菜单事件,name=")[1]
            menu_fun.append({"name":name,"function":fun})
        else:
            t = int(value.split(",type=")[1])
            event_fun.append({"type":t,"priority":20000,"function":fun,"id":tid,"name":value.split(",type=")[0]})
            tid = tid + 1
    url = str(inputText.get()).strip()
    if url == '':
        tk.messagebox.showerror(title="生成失败",message="请选择一个DLL")
        return
    (path, filename) = os.path.split(url)
    (file, ext) = os.path.splitext(filename)
    js = {
        "name":file,
        "ret": 1,
        "apiver": 9,
         "version":"1.0.0",
         "author":"xxx",
         "description":"这是一个插件",
    }
    if len(event_fun) > 0:
        js['event'] = event_fun
    if len(menu_fun) > 0:
        js['menu'] = menu_fun
    json_str = json.dumps(js,ensure_ascii=False,indent=4).encode('gbk')

    try:
        with open(file+".json",'wb+') as f:
            f.write(json_str)
        tk.messagebox.showinfo(title="生成成功",message="生成成功:" + file+".json")
    except:
        tk.messagebox.showerror(title="生成失败",message="文件写入失败")
        


saveButton = tk.Button(window, height=1, text="保存JSON", bg='Lavender', command = save_json)
saveButton.grid(row=2, column=0,columnspan=2,padx=8,pady=4, sticky=tk.W + tk.E )

def choose_dll_clicked():
    fn = tk.filedialog.askopenfilename(filetypes=[('DLL', '*.dll')])
    if fn != '':
        x=treeview.get_children()
        for item in x:
            treeview.delete(item)
        buf = create_string_buffer(''.encode(), 90000)
        ret = cdll.dllfunview.DllFunView(fn.encode(),buf,90000)
        if ret != 0:
            tk.messagebox.showerror(title="DLL加载出错",message="错误信息:" + str(ret))
            return
        funVec = string_at(buf).decode().split(";")
        if "Initialize" not in funVec:
            tk.messagebox.showerror(title="DLL加载出错",message="你选择的不是MiraiCQ插件，无Initialize函数")
            return
        name = [i for i in funVec if "@" not in i and i != 'AppInfo' and i != 'Initialize']
        for i in range(len(name)):
             treeview.insert('', i, values=(name[i], "空事件"))
        inputText["state"] = "normal"
        inputText.delete("0", 'end')
        inputText.insert(tkinter.END, str(fn))
        inputText["state"] = "readonly"

inputButton = tk.Button(window, height=1, text="选择DLL", bg='Lavender',command = choose_dll_clicked)
inputButton.grid(row=0, column=1, padx=8,pady=4,sticky=tk.W)

def set_cell_value(event):
    if treeview.identify_row(event.y) == '':
        return
    v = choose_event(window)
    treeview.set(treeview.identify_row(event.y),1,value = v)
treeview.bind('<Double-1>', set_cell_value)

window.mainloop()
