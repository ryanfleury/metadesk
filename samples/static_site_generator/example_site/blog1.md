title: "Test Blog #1"
desc:  "This is my test blog."
date:  (2020/11/30)
index: {"blog"}
parent: "blog_main"

@subtitle "Section 1"

"""
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse ut ornare neque, vitae finibus leo. Praesent pulvinar, urna id lobortis iaculis, velit turpis luctus tortor, quis malesuada neque neque vitae turpis. Phasellus id molestie elit. Sed elementum ipsum a ligula semper, at varius augue rutrum. Pellentesque fringilla, orci nec fringilla finibus, turpis lacus vehicula elit, nec bibendum magna est quis massa. Aliquam efficitur facilisis nibh, ut sodales sapien scelerisque nec. Integer justo sem, pellentesque et ante imperdiet, interdum placerat sem.

Nunc hendrerit lobortis commodo. Morbi felis quam, fermentum vitae libero vitae, fermentum sodales quam. Nulla bibendum tellus quis lorem sollicitudin pretium. Etiam commodo ex eget aliquet porta. In sit amet dui eleifend, mattis sem nec, tristique erat. Morbi malesuada fringilla bibendum. Integer odio ex, cursus a sodales quis, vehicula non dolor.

Praesent vitae pharetra felis. Curabitur non ex non nunc pretium feugiat eu sit amet turpis. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Aenean blandit ante leo. Vivamus iaculis mi vel sem tincidunt maximus. Aenean elementum ipsum non laoreet semper. Morbi non felis interdum lectus pulvinar mollis id non purus. Nullam eu ipsum ut turpis aliquam feugiat at non tellus. Suspendisse ornare erat quis enim ullamcorper, at rhoncus nulla suscipit.
"""

@img {"test.png", "My funny image."}

@subtitle "Section 2"

"""
Aliquam quis diam at sem interdum imperdiet. Nunc imperdiet ligula tempus nibh semper mattis. Suspendisse fringilla molestie semper. Sed felis dolor, vehicula et tempus sed, porttitor finibus lacus. Pellentesque egestas ex finibus, facilisis tellus suscipit, accumsan turpis. Ut imperdiet vitae nisl eget vestibulum. Donec eu bibendum erat, id maximus magna. Vivamus quis rhoncus justo. Morbi eget commodo lorem, vulputate varius ipsum. Interdum et malesuada fames ac ante ipsum primis in faucibus. Integer accumsan, sem vel rutrum cursus, odio enim sodales leo, sit amet rutrum odio purus at nisi. Fusce iaculis ante id nunc volutpat tincidunt. Duis nec tincidunt ipsum. Fusce vitae odio ac velit sollicitudin maximus. Vestibulum ante dui, varius auctor ante eu, gravida laoreet justo.

Vivamus dignissim mauris nec turpis convallis, vitae pellentesque sapien faucibus. Phasellus eu euismod elit, sit amet vehicula diam. Curabitur sit amet leo magna. In est erat, congue vel euismod id, venenatis vitae mauris. Vestibulum sit amet leo eget leo ornare feugiat. Interdum et malesuada fames ac ante ipsum primis in faucibus. Vivamus at efficitur ex, a mattis lorem. Maecenas blandit tincidunt tortor vel cursus. In in purus a nisi pretium venenatis in ut lacus. Maecenas venenatis rutrum laoreet. Donec aliquam leo vel risus cursus, placerat pretium lacus fringilla. Suspendisse ut porttitor mauris.
"""

@youtube {"BjdY1eXB6cY"}

@subtitle "Section 3"

@list
{
    "hello",
    "world",
    "foo",
    @list
    {
        "a",
        "b",
        "c",
    },
    "another list item",
}

@subtitle "Section 4"

@code
"""
char *html_tag = "p";
if(DD_NodeHasTag(node, DD_S8Lit("title")))
{
    html_tag = "h1";
}
else if(DD_NodeHasTag(node, DD_S8Lit("subtitle")))
{
    html_tag = "h2";
}
else if(DD_NodeHasTag(node, DD_S8Lit("code")))
{
    html_tag = "pre";
}
"""