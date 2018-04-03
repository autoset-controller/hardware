<template>
  <section id="design">
    <div class="top-section">
      <cues></cues>
      <preview></preview>
    </div>
    <configuration></configuration>
  </section>
</template>

<script>
import Cues from './Design/Cues';
import Configuration from './Design/Configuration';
import Preview from './Design/Preview';

export default {
  components: {
    Cues,
    Configuration,
    Preview
  }
};
</script> 

<style>
@import url('https://fonts.googleapis.com/css?family=Source+Sans+Pro');
html, body, div, span, applet, object, iframe,
h1, h2, h3, h4, h5, h6, p, blockquote, pre,
a, abbr, acronym, address, big, cite, code,
del, dfn, em, img, ins, kbd, q, s, samp,
small, strike, strong, sub, sup, tt, var,
b, u, i, center,
dl, dt, dd, ol, ul, li,
fieldset, form, label, legend,
table, caption, tbody, tfoot, thead, tr, th, td,
article, aside, canvas, details, embed, 
figure, figcaption, footer, header, hgroup, 
menu, nav, output, ruby, section, summary,
time, mark, audio, video {
	margin: 0;
	padding: 0;
	border: 0;
	vertical-align: baseline;
}
h1, h2, h3, h4 {
    letter-spacing: 0rem;
}
h3 {
    font-size: 18px;
}

.top-bar {
    padding-bottom: 10px;
}
.top-bar p {
    text-align: center;
    margin: 0;
}

#design{
    display: grid;
    grid-template-rows: 2fr 1fr;
    flex-grow: 1;
}

.top-section{
    display: grid;
    grid-template-columns: 1fr 1fr;
}
.cues, table.table{
    background: #1C1C1C;
    color: white;
    width: 100%;
}
.table thead th{
    color: white;
}
td { 
    cursor: pointer;
    padding: 5px 0;
}
tr:hover {
    background: black;
}
tr.active { background: black; }

input {
    color: white;
    margin-right: 10px;
}

.form-row {
    display: flex;
    padding: 0 20px;
}
.formElement {
    padding: 0 10px;
}
</style>

