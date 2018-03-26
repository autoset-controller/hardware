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
body{
    height: 100vh;
    margin: 0;
    background: #1C1C1C;
    color: white;
    display: flex;
    flex-direction: column;
    font-family: 'Source Sans Pro', sans-serif;
    font-size: 16px;
}
h1, h2, h3, h4 {
    letter-spacing: 0rem;
}
h3 {
    font-size: 18px;
}

button {
    outline: none;
    border: 0;
    padding: 5px;
    font-size: 1.5rem;
    margin: 5px;
    border-radius: 2px;
    cursor: pointer;
}

.top-bar {
    padding-bottom: 10px;
}
.top-bar p {
    text-align: center;
    margin: 0;
}
nav {
    text-align: center;
    /* border-bottom: 1px solid white; */
}
nav .nav-item{
    display: inline;
    font-size: 2rem;
    margin: 20px;
    cursor: pointer;
}
nav .nav-item.active {
    text-shadow: 0px 2px 5px white;
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
button.submit {
    background: #2ecc71;
    color: white;
}

.form-row {
    display: flex;
    padding: 0 20px;
}
.formElement {
    padding: 0 10px;
}
</style>

