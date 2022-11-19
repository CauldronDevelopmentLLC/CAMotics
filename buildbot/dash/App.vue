<script>


export default {
  data() {
    return {
      pattern: '',
      _builders: []
    }
  },


  computed: {
    builders() {
      let l = []

      try {
        let builders = this._builders
        let re = new RegExp(this.pattern)

        for (let i = 0; i < builders.length; i++)
          if (builders[i].masterids.length)
            if (!this.pattern || re.test(builders[i].name))
              l.push(builders[i])
      } catch (e) {}

      return l
    }
  },


  mounted() {
    fetch('/api/v2/builders')
      .then(r => r.json())
      .then(data => this._builders = data.builders)
  },


  methods: {
    post(url, data) {
      fetch(url, {
        method: 'POST',
        headers: {
          'Accept': 'application/json',
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      })
    },


    force(builder) {
      if (builder == undefined)
        for (let i = 0; i < this.builders.length; i++)
          this.force(this.builders[i])

      else {
        let data = {
          id:      1,
          jsonrpc: '2.0',
          method:  'force',
          params: {
            builderid:  builder.builderid,
            username:   '',
            reason:     'force build',
            branch:     '',
            project:    '',
            repository: '',
            revision:   ''
          }
        }

        this.post('/api/v2/forceschedulers/' + builder.name, data)
      }
    },


    cancel_build_request(id) {
      let data = {
        id:      1,
        jsonrpc: '2.0',
        method:  'cancel',
        params: {}
      }

      this.post('/api/v2/buildrequests/' + id, data)
    },


    cancel(builder) {
      if (builder == undefined)
        for (let i = 0; i < this.builders.length; i++)
          this.cancel(this.builders[i])

      else {
        fetch('/api/v2/builders/' + builder.builderid +
              '/buildrequests?claimed=false')
          .then(r => r.json())
          .then(data => {
            let reqs = data.buildrequests

            for (let i = 0; i < reqs.length; i++)
              this.cancel_build_request(reqs[i].buildrequestid)
          })
      }
    }
  }
}
</script>

<template lang="pug">
header
  h1 Force Builders

main
  table
    tr
      th Builder
      th Action

    tr
      th.search
        div Search
        input(v-model="pattern")
      td
        button(@click="force()") Force All
        button(@click="cancel()") Cancel All

    tr(v-for="b in builders")
      td: a(:href="'#builders/' + b.builderid") {{b.name}}
      td
        button(@click="force(b)") Force
        button(@click="cancel(b)") Cancel
</template>

<style lang="stylus">
#force-dash-app
  padding 2em
  padding-top 0

  table
    border-collapse collapse

  th
    font-weight bold
    font-size 120%

    &.search
      display flex
      gap 0.5em

  th, td
    padding 0.125em 0.5em

    input
      width 100%
      font-weight normal

    button
      width 8em
      white-space nowrap
</style>
