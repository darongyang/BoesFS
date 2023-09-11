

https://linux.die.net/man/5/acl


以下为机翻

## Name

acl - Access Control Lists

## Description

本手册页介绍了 POSIX 访问控制列表，这些列表用于定义文件和目录的更细粒度的任意访问权限。


## Acl Types

可以将每个对象视为具有与之关联的 ACL，该 ACL 控制对该对象的任意访问;此 ACL 称为访问 ACL。此外，目录可能具有关联的 ACL，用于控制在该目录中创建的对象的初始访问 ACL;此 ACL 称为默认 ACL。

## Acl Entries

ACL 由一组 ACL 条目组成。ACL 条目将单个用户或一组用户的关联对象的
访问权限指定为 读取、写入 和 搜索/执行 权限的组合。

ACL 条目包含条目标记类型、可选条目标记限定符和一组权限。我们使用术语限定符来表示 ACL 条目的条目标记限定符。

限定符表示用户或组的标识符，分别用于标记类型为 ACL_USER 或 ACL_GROUP 的条目。标记类型不是 ACL_USER 或 ACL_GROUP 的条目没有定义的限定符。


定义了以下条目标记类型：

- ACL_USER_OBJ : The ACL_USER_OBJ entry denotes access rights for the file owner.

- ACL_USER     :   ACL_USER entries denote access rights for users identified by the entry's qualifier.

- ACL_GROUP_OBJ :  The ACL_GROUP_OBJ entry denotes access rights for the file group.

- ACL_GROUP    :   ACL_GROUP entries denote access rights for groups identified by the entry's qualifier.

- ACL_MASK   :     The ACL_MASK entry denotes the maximum access rights that can be granted by entries of type ACL_USER, ACL_GROUP_OBJ, or ACL_GROUP.

- ACL_OTHER   :    The ACL_OTHER entry denotes access rights for processes that do not match any other entry in the ACL.

执行访问检查时，将根据有效用户 ID 测试ACL_USER_OBJ和ACL_USER条目。有效组 ID 以及所有补充组 ID 将针对ACL_GROUP_OBJ和ACL_GROUP条目进行测试。


## VALID ACLs

有效的 ACL 只包含一个条目，其中包含每个 ACL_USER_OBJ、ACL_GROUP_OBJ 和ACL_OTHER标记类型。具有 ACL_USER 和ACL_GROUP标记类型的条目可能在 ACL 中出现零次或多次。包含 ACL_USER 或 ACL_GROUP 标记类型的条目的 ACL 必须只包含一个ACL_MASK标记类型的条目。如果 ACL 不包含 ACL_USER 或 ACL_GROUP 标记类型的条目，则ACL_MASK条目是可选的。

所有用户 ID 限定符在ACL_USER标记类型的所有条目中必须是唯一的，并且所有组 ID 在ACL_GROUP标记类型的所有条目中必须是唯一的。

如果目录未与默认 ACL 关联，则 acl_get_file（） 函数返回具有零个 ACL 条目的 ACL 作为目录的默认 ACL。acl_set_file（） 函数还接受具有零个 ACL 条目的 ACL 作为目录的有效默认 ACL，表示该目录不应与默认 ACL 关联。这等效于 使用 acl_delete_def_file（） 函数。

## CORRESPONDENCE BETWEEN ACL ENTRIES AND FILE PERMISSION BITS

ACL 定义的权限是文件权限位指定的权限的超集。

文件所有者、组和其他权限与特定 ACL 条目之间存在对应关系：所有者权限对应于ACL_USER_OBJ条目的权限。如果 ACL 具有ACL_MASK条目，则组权限对应于ACL_MASK条目的权限。否则，如果 ACL 没有ACL_MASK条目，则组权限对应于ACL_GROUP_OBJ条目的权限。其他权限对应于ACL_OTHER_OBJ条目的权限。

文件所有者、组和其他权限始终与相应 ACL 条目的权限匹配。修改文件权限位会导致修改关联的 ACL 条目，修改这些 ACL 条目会导致修改文件权限位。

## OBJECT CREATION AND DEFAULT ACLs

当使用任何 creat（）、mkdir（）、mknod（）、mkfifo（） 或 open（） 函数创建对象时，将初始化文件对象的访问 ACL。如果默认 ACL 与目录关联，则创建文件对象的函数的 mode 参数和目录的默认 ACL 用于确定新对象的 ACL：

1. 新对象继承包含目录的默认 ACL 作为其访问 ACL。
2. 修改与文件权限位对应的访问 ACL 条目，使其不包含 mode 参数指定的权限中未包含的权限。

如果没有缺省 ACL 与目录关联， 则创建文件对象的函数的 mode 参数和文件创建掩码 （参见 umask（2）） 用于确定新对象的 ACL：

1. 为新对象分配一个访问 ACL，其中包含标记类型 ACL_USER_OBJ、ACL_GROUP_OBJ 和 ACL_OTHER 的条目。这些条目的权限设置为文件创建掩码指定的权限。

2. 修改与文件权限位对应的访问 ACL 条目，使其不包含 mode 参数指定的权限中未包含的权限。

## ACCESS CHECK ALGORITHM

进程可以请求对受 ACL 保护的文件对象的读取、写入或执行/搜索访问权限。访问检查算法确定是否授予对对象的访问权限。

```c

     1.   If the effective user ID of the process matches the user ID of the file object owner, then

                if the ACL_USER_OBJ entry contains the requested permissions, access is granted,

                else access is denied.

     2.   else if the effective user ID of the process matches the qualifier of any entry of type ACL_USER, then

                if the matching ACL_USER entry and the ACL_MASK entry contain the requested permissions, access is granted,

                else access is denied.

     3.   else if the effective group ID or any of the supplementary group IDs of the process match the file group or the qualifier of any entry of type
          ACL_GROUP, then

                if the ACL contains an ACL_MASK entry, then

                      if  the  ACL_MASK  entry  and  any of the matching ACL_GROUP_OBJ or ACL_GROUP entries contain the requested permissions, access is
                      granted,

                      else access is denied.

                else (note that there can be no ACL_GROUP entries without an ACL_MASK entry)

                      if the ACL_GROUP_OBJ entry contains the requested permissions, access is granted,

                      else access is denied.

     4.   else if the ACL_OTHER entry contains the requested permissions, access is granted.

     5.   else access is denied.
```


## ACL TEXT FORMS

定义了用于表示 ACL 的长文本形式和短文本形式。在这两种形式中，ACL 条目都表示为三个冒号分隔的字段：ACL 条目标签类型、ACL 条目限定符和任意访问权限。

第一个字段包含以下条目标记类型关键字之一：
```txt
           user    A user ACL entry specifies the access granted to either the file owner (entry tag type ACL_USER_OBJ) or a specified user (entry tag
                   type ACL_USER).

           group   A group ACL entry specifies the access granted to either the file group (entry tag type ACL_GROUP_OBJ) or a specified group (entry
                   tag type ACL_GROUP).

           mask    A mask ACL entry specifies the maximum access which can be granted by any ACL entry except the user entry for the file owner and the
                   other entry (entry tag type ACL_MASK).

           other   An other ACL entry specifies the access granted to any process that does not match any user or group ACL entries (entry tag type
                   ACL_OTHER).
```

第二个字段包含与条目标记类型为 ACL_USER 或 ACL_GROUP 的条目的 ACL 条目关联的用户或组的用户或组标识符，对于所有其他条目为空。用户标识符可以是十进制形式的用户名或用户 ID 号。组标识符可以是十进制形式的组名称或组 ID 号。

第三个字段包含自由访问权限。读取、写入和搜索/执行权限按此顺序由 r、w 和 x 字符表示。其中每个字符都替换为 - 字符，以表示 ACL 条目中没有权限。从文本形式转换为内部表示形式时，无需指定不存在的权限。

允许在每个 ACL 条目的开头和结尾以及字段分隔符（冒号字符）之前和之后使用空格。


### LONG TEXT FORM

长文本表单每行包含一个 ACL 条目。此外，数字符号 （#） 可能会开始延伸至行尾的注释。如果 ACL_USER、ACL_GROUP_OBJ 或ACL_GROUP ACL 条目包含的权限未包含在ACL_MASK条目中，则该条目后跟数字符号、字符串“valid：”以及该条目定义的有效访问权限。这是长文本表单的示例：
```txt
           user::rw-
           user:lisa:rw-         #effective:r--
           group::r--
           group:toolies:rw-     #effective:r--
           mask::r--
           other::r--
```    

### SHORT TEXT FORM


短文本形式是一系列以逗号分隔的 ACL 条目，用于输入。不支持注释。条目标签类型关键字可以以其完整的非缩写形式出现，也可以以单个字母缩写形式出现。用户的缩写是u，组的缩写是g，掩码的缩写是m，其他的缩写是o。权限最多可以包含以下每个字符（按任意顺序排列）：r、w、x。以下是短文本形式的示例：
```txt
           u::rw-,u:lisa:rw-,g::r--,g:toolies:rw-,m::r--,o::r--
           g:toolies:rw,u:lisa:rw,u::wr,g::r,o::r,m::r
```


## Rational

IEEE 1003.1e 草案 17 定义了访问控制列表，其中包括标记类型 ACL_MASK 的条目，并定义了文件权限位之间的映射，该映射不是恒定的。标准工作组定义了这个相对复杂的接口，以确保符合IEEE 1003.1（“POSIX.1”）的应用程序在具有ACL的系统上仍按预期运行。IEEE 1003.1e 草案 17 包含第 B.23 节中选择此接口的基本原理。


## CHANGES TO THE FILE UTILITIES


在支持 ACL 的系统上，文件实用程序 ls（1）、cp（1） 和 mv（1） 按以下方式更改它们的行为：

对于具有缺省 ACL 或包含三个以上必需 ACL 条目的访问 ACL 的文件， 由 ls -l 生成的长格式的 ls（1） 实用程序在权限字符串后显示一个加号 （+）

如果指定了 -p 标志， cp（1） 工具也会保留 ACL。 如果无法做到这一点，则会生成警告

mv（1） 工具总是保留 ACL。如果无法做到这一点，则会生成警告


## Standards

IEEE 1003.1e 草案 17（“POSIX.1e”）文档描述了 IEEE 1003.1 标准的几个安全扩展。虽然 1003.1e 的工作已被放弃，但许多 UNIX 风格的系统实现了 POSIX.1e 草案 17 或早期草案的部分内容。

Linux 访问控制列表实现了为 POSIX.1e 中的访问控制列表定义的全套功能和实用程序，以及多个扩展。实施完全符合 POSIX.1e 草案 17;扩展名被标记为这样。访问控制列表操作函数在 ACL 库 （libacl， -lacl） 中定义。POSIX 兼容接口在 <sys/acl.h> 标头中声明。这些函数的特定于 Linux 的扩展在 <acl/libacl.h> 标头中声明。


## See Also

chmod(1), creat(2), getfacl(1), ls(1), mkdir(2), mkfifo(2), mknod(2), open(2), setfacl(1), stat(2), umask(1)


...略